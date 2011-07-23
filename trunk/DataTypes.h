/*
This file contains basic data types used in asfermi.
0: SubString, defined in SubString.h
1: Basic structures used by the assembler: Component, Line, Instruction, Directive
2: Structures for line analysis: ModifierRule, OperandRule, InstructionRule
3: Abstract parser structures: Parser, MasterParser, LineParser, InstructionParser, DirectiveParser
*/

#ifndef DataTypesDefined //prevent multiple inclusion
//---code starts ---





using namespace std;

//-----Extern declarations
extern char* csSource; //The array with the entire source file stored in it
extern int csMaxReg;   //highest register used
extern int hpParseComputeInstructionNameIndex(SubString &name);// compute instruction index from instruction name
extern int hpParseComputeDirectiveNameIndex(SubString &name);
//-----End of extern declaration

//	1
//-----Basic structures used by the assembler: Line, Instruction, Directive

struct Line
{
	SubString LineString;
	int LineNumber;
	Line(){}
	Line(SubString lineString, int lineNumber)
	{
		LineString = lineString;
		LineNumber = lineNumber;
	}
};
struct Instruction
{
	SubString InstructionString;
	int LineNumber;
	list<SubString> Modifiers;
	list<SubString> Components; //predicate is the first optional component. Then instruction name (without modifier), then operands(unprocesed, may contain modifier)
	bool Is8;	//true: 8-byte opcode. OpcodeWord1 is used as well
	unsigned int OpcodeWord0;
	unsigned int OpcodeWord1;
	int Offset;	//Instruction offset in assembly
	bool Predicated;//indicates whether @Px is present at the beginning
	
	Instruction(){}
	Instruction(SubString instructionString, int offset, int lineNumber)
	{
		InstructionString = instructionString;
		Offset = offset;
		LineNumber = lineNumber;
	}
	void Reset(SubString instructionString, int offset, int lineNumber)
	{
		InstructionString = instructionString;
		Offset = offset;
		LineNumber = lineNumber;
		Components.clear();
		Is8 = true;
		OpcodeWord0 = 0;
		OpcodeWord1 = 0;
		Predicated = false;
	}
};
struct Directive
{
	SubString DirectiveString;
	int LineNumber;
	list<SubString> Parts; //Same as Components in Instruction
	Directive(){}
	Directive(SubString directiveString, int lineNumber)
	{
		DirectiveString = directiveString;
		LineNumber = lineNumber;
	}
	void Reset(SubString directiveString, int lineNumber)
	{
		DirectiveString = directiveString;
		LineNumber = lineNumber;
		Parts.clear();
	}
};
//-----End of basic types





//	2
//-----Structures for line analysis: ModifierRule, OperandRule, InstructionRule, DirectiveRule
//this enum is largely useless for now
typedef enum OperandType
{
	Register, Immediate32HexConstant, Predicate,
	Immediate32IntConstant, Immediate32FloatConstant, Immediate32AnyConstant, 
	GlobalMemoryWithImmediate32, ConstantMemory, SharedMemoryWithImmediate20, Optional, Custom, 
	MOVStyle, FADDStyle, IADDStyle
};

//Rule for specific modifier
struct ModifierRule
{
	SubString Name;// .RZ would have a name of RZ

	bool Apply0; //apply on OpcodeWord0?
	unsigned int Mask0; // Does an AND operation with opcode first
	unsigned int Bits0; //then an OR operation

	bool Apply1; //Apply on OpcodeWord1?
	unsigned int Mask1;
	unsigned int Bits1;

	bool NeedCustomProcessing;
	virtual void CustomProcess(){}
	ModifierRule(){}
	ModifierRule(char* name, bool apply0, bool apply1, bool needCustomProcessing)
	{
		Name = name;
		Apply0 = apply0;
		Apply1 = apply1;
		NeedCustomProcessing = needCustomProcessing;
	}
};

//Modifiers are grouped. Modifiers must be present in the correct sequence in which modifier groups are arranged
//Different modifiers from the same group cannot appear at the same time
//For example, FMUL has 2 modifier groups
//1st group: .RP/.RM/.RZ
//2nd group: .SAT
//So if .SAT is to be present, it must be the last modifier
//Only one modifier from the first group could be present
struct ModifierGroup
{
	int ModifierCount; //number of possible modifiers there are in this group
	ModifierRule**  ModifierRules; //point to the modifier rules
	bool Optional; //whether this modifier group is optional
	~ModifierGroup()
	{
		if(ModifierCount)
			delete[] ModifierRules;
	}
	void Initialize(bool optional, int modifierCount, ...)
	{
		Optional = optional;
		va_list modifierRules;
		va_start (modifierRules, modifierCount);
		ModifierCount = modifierCount;
		ModifierRules = new ModifierRule*[modifierCount];
		for(int i =0; i<modifierCount; i++)
			ModifierRules[i] = va_arg(modifierRules, ModifierRule*);
		va_end(modifierRules);

	}
};

//Rule for processing specific operand
struct OperandRule
{
	OperandType Type;//not really useful. 
	//bool Optional;

	OperandRule(){}
	OperandRule(OperandType type)
	{
		Type = type;
	}
	//the custom processing function used to process a specific operand(component) that must be defined in child classes
	virtual void Process(SubString &component) = 0;
};

//When an instruction rule is initialized, the ComputeIndex needs to be called. 
//They need to be sorted according to their indices and then placed in csInstructionRules
struct InstructionRule
{
	char* Name;
	int OperandCount;
	OperandRule** Operands;

	int ModifierGroupCount;
	ModifierGroup *ModifierGroups;
	

	bool Is8;
	unsigned int OpcodeWord0;
	unsigned int OpcodeWord1;

	//If NeedCustomProcessing is set to true, the components of an instruction SubString
	//will not be processed according to the operand rules. Instead, the CustomProcess function
	//will be called
	bool NeedCustomProcessing;
	virtual void CustomProcess(){}
	int ComputeIndex()
	{
		int result = 0;
		int len = strlen(Name);
		SubString nameString;
		nameString.Start = Name;
		nameString.Length = len;
		result = hpParseComputeInstructionNameIndex(nameString);
		return result;
	}
	InstructionRule(){};
	InstructionRule(char* name, int modifierGroupCount, bool is8, bool needCustomProcessing)
	{
		Name = name;
		OperandCount = 0;
		ModifierGroupCount = modifierGroupCount;
		if(modifierGroupCount>0)
			ModifierGroups = new ModifierGroup[modifierGroupCount];
		Is8 = is8;
		NeedCustomProcessing = needCustomProcessing;
	}
	void SetOperands(int operandCount, ...)
	{
		OperandCount = operandCount;
		Operands = new OperandRule*[operandCount];
		va_list operandRules;
		va_start (operandRules, operandCount);
		for(int i =0; i<operandCount; i++)
			Operands[i] = va_arg(operandRules, OperandRule*);
		va_end(operandRules);

	}
	~InstructionRule()
	{
		if(OperandCount)
			delete[] Operands;
		if(ModifierGroupCount)
			delete[] ModifierGroups;		
	}
};

struct DirectiveRule
{
	char* Name;

	virtual void Process() = 0;
	int ComputeIndex()
	{
		int result = 0;
		int len = strlen(Name);
		SubString nameString;
		nameString.Start = Name;
		nameString.Length = len;
		result = hpParseComputeDirectiveNameIndex(nameString);
		return result;
	}
};
//-----End of structures for line analysis






//	3
//-----Abstract parser structures: Parser, MasterParser, LineParser, InstructionParser, DirectiveParser
struct Parser
{
	char* Name;
};
struct MasterParser: Parser
{
	virtual void Parse(unsigned int startinglinenumber) = 0;
};
struct LineParser: Parser
{
	virtual void Parse(Line &line) = 0;
};
struct InstructionParser: Parser
{
	virtual void Parse() = 0;
};
struct DirectiveParser: Parser
{
	virtual void Parse() = 0;
};
//-----End of abstract parser structures








//	9.0
//-----Label structures
struct Label
{
	string Name;
	int Offset;
};
struct LabelRequest
{
	Instruction *RelatedInstruction;
	string RequestedLabelName;
};
//-----End of Label structures
#else
#define DataTypesDefined
#endif