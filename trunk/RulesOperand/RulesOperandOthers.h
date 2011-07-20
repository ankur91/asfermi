#ifndef RulesOperandOthersDefined



//ignored operand: currently used for NOP
struct OperandRuleIgnored: OperandRule
{
	OperandRuleIgnored() : OperandRule(OperandType::Optional){}
	virtual void Process(SubString &component)
	{
		//do nothing
	}
}OPRIgnored;




struct OperandRule32I: OperandRule
{
	OperandRule32I() : OperandRule(Custom){}
	virtual void Process(SubString &component)
	{
		unsigned int result;
		if(component[0]=='F')
		{
			result = component.ToImmediate32FromFloatConstant();
			goto write;
		}
		int startPos = 0;
		if(component[0]=='-')
			startPos=1;
		if(component.Length-startPos>2 && component[startPos] == '0' && (component[startPos+1]=='x' || component[startPos+1]=='X'))
		{
			result = component.ToImmediate32FromHexConstant(true);
		}
		else
		{
			result = component.ToImmediate32FromIntConstant();
		}
		write:
		WriteToImmediate32(result);
	}
}OPR32I;




struct OperandRuleLOP: OperandRule
{
	int ModShift;
	OperandRuleLOP(int modShift): OperandRule(Custom)
	{
		ModShift = modShift;
	}
	virtual void Process(SubString &component)
	{
		bool not = false;
		if(component[0]=='~')
		{
			not = true;
			component.Start++;
			component.Length--;
		}
		if(component.Length<1)
			throw 132; //empty operand
		if(ModShift==8)
			OPRMOVStyle.Process(component);
		else
			OPRRegister1.Process(component);
		if(not)
		{
			csCurrentInstruction.OpcodeWord0 |= 1<<ModShift;
			component.Start--;
			component.Length++;
		}
	}
}OPRLOP1(9), OPRLOP2(8);


struct OperandRuleF2I: OperandRule
{
	bool F2I;
	OperandRuleF2I(bool f2I): OperandRule(Custom)
	{
		F2I = f2I;
	}
	virtual void Process(SubString &component)
	{
		bool operated = false;
		if(component[0]=='-')
		{
			operated = true;
			csCurrentInstruction.OpcodeWord0 |= 1<<8;
			component.Start++;
			component.Length--;
		}
		else if(component[0]=='|')
		{
			operated = true;
			csCurrentInstruction.OpcodeWord0 |= 1<<6;
			component.Start++;
			component.Length--;
		}
		if(F2I)
			OPRFMULStyle.Process(component);
		else
			OPRIMULStyle.Process(component);
		if(operated)
		{
			component.Start--;
			component.Length++;
		}
	}
}OPRF2I(true), OPRI2F(false);


#else
#define RulesOperandOthersDefined
#endif