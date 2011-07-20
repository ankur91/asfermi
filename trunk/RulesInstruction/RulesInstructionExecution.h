#ifndef RulesInstructionExecutionDefined


struct InstructionRuleEXIT: InstructionRule
{
	InstructionRuleEXIT() : InstructionRule("EXIT", 0, true, false)
	{
		InstructionRule::BinaryStringToOpcode8("1110011110111000000000000000000000000000000000000000000000000001", OpcodeWord0, OpcodeWord1);
	}
}IREXIT;

struct InstructionRuleCAL: InstructionRule
{
	InstructionRuleCAL() : InstructionRule("CAL", 1, true, false)
	{
		InstructionRule::BinaryStringToOpcode8("1110000000000000100000000000000000000000000000000000000000001010", OpcodeWord0, OpcodeWord1);
		SetOperands(1, &OPRImmediate24HexConstant);
		ModifierGroups[0].Initialize(true, 1, &MRCALNOINC);
	}
}IRCAL;

struct InstructionRuleBRA: InstructionRule
{
	InstructionRuleBRA() : InstructionRule("BRA", 1, true, false)
	{
		InstructionRule::BinaryStringToOpcode8("1110011110111000000000000000000000000000000000000000000000000010", OpcodeWord0, OpcodeWord1);
		SetOperands(1, &OPRImmediate24HexConstant);
		ModifierGroups[0].Initialize(true, 1, &MRBRAU);
	}
}IRBRA;

struct InstructionRulePRET: InstructionRule
{
	InstructionRulePRET() : InstructionRule("PRET", 1, true, false)
	{
		InstructionRule::BinaryStringToOpcode8("1110000000000001000000000000000000000000000000000000000000011110", OpcodeWord0, OpcodeWord1);
		SetOperands(1, &OPRImmediate24HexConstant);
		ModifierGroups[0].Initialize(true, 1, &MRCALNOINC);
	}
}IRPRET;



struct InstructionRuleRET: InstructionRule
{
	InstructionRuleRET() : InstructionRule("RET", 0, true, false)
	{
		InstructionRule::BinaryStringToOpcode8("1110011110111000000000000000000000000000000000000000000000001001", OpcodeWord0, OpcodeWord1);
	}
}IRRET;

#else
#define RulesInstructionExecutionDefined
#endif