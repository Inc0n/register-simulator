
#include "instruction-set.hpp"
#include "assembler.hpp"

//

Param::Param() : type(tagtype::Nil), value(0) {}

Param::Param(tagtype type, const char* label) :
    type(type),
    label(strdup(label))
{ // can only be label
    assert(type == tagtype::Label);
}

Param::Param(tagtype type, int_t value) :
    type(type), value(value)
{ // cannot be label
    assert(type != tagtype::Label);
}

size_t Param::size() {
    return (type == tagtype::Nil) ? 0 : 2;
}

bool Param::operator==(const Param& p) const
{
    return this->type == p.type
        && (this->type == tagtype::Label
            ? strcmp(this->label, p.label) == 0
            : this->value == p.value);
}


// Instruction

Instruction::Instruction(INST inst, std::vector<Param> params):
    inst(inst), params(params)
{}

Instruction::Instruction(INST inst, Param param1, Param param2) :
    Instruction(inst, { param1, param2 })
{
    // params[0] = param1;
    // params[1] = param2;
}

Instruction::Instruction(INST inst, Param param1) :
    Instruction(inst, { param1, Param(tagtype::Nil, (int_t)0) })
{}

Instruction::Instruction() :
    Instruction(INST::NOP, { Param(), Param() })
{}

bool Instruction::operator==(const Instruction& inst) const {
    return this->inst == inst.inst
        && params[0] == inst.params[0]
        && params[1] == inst.params[1];
 }

//

std::string tagtypeString(tagtype type) {
    switch (type) {
    case tagtype::Label: return "label";
    case tagtype::LabelIdx: return "labelIdx";
    case tagtype::Reg:   return "reg";
    case tagtype::Mem:   return "mem";
    case tagtype::Const: return "const";
    case tagtype::Nil:   return "nil";
    default: return "ERROR";
    }
}

std::ostream& operator<<(std::ostream& os, const Param& p) {
	os << tagtypeString(p.gettype()) << ".";
	if (p.gettype() == tagtype::Label)
		os << p.getlabel();
	else
		os << p.getvalue();
	return os;
}


std::string instString(INST inst) {
    const operator_t *op = getOperator(inst);
    return op ? op->name : "ERROR";
}

std::ostream& operator<<(std::ostream& os, const Instruction& i) {
	return os << instString(i.getInstCode())
			  << " " << i.params[0]
			  << " " << i.params[1];
}

