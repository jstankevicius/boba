#include "bytecode.h"

namespace bytecode {

    // push always requires an argument, so it needs to take an AST node from
    // which it can derive the argument.
    Instruction push(std::shared_ptr<AST> ast) {

        Instruction inst(InstructionType::PUSH);
        std::string s = ast->string_value;

        switch (ast->type) {
            case AST_STR_LITERAL:
                inst.value = make_value(ValueType::STRING, s);
                break;
            case AST_INT_LITERAL:
                inst.value = make_value(ValueType::INT, std::stoi(s));
                break;
            case AST_FLOAT_LITERAL:
                inst.value = make_value(ValueType::FLOAT, std::stof(s));
                break;
            case AST_BOOL_LITERAL:
                // The string value is guaranteed to be either "true" or "false"
                // by the parser.
                inst.value = make_value(ValueType::BOOL, s == "true");
                break;
            case AST_SYMBOL:
                inst.value = make_value(ValueType::SYMBOL, s);
                break;  
            default:
                break;
        }
        return inst;
    }

    Instruction add(int n_args) {
        Instruction inst(InstructionType::ADD);
        inst.value = make_value(ValueType::INT, n_args);
        return inst;
    }

    Instruction sub(int n_args) {
        Instruction inst(InstructionType::SUB);
        inst.value = make_value(ValueType::INT, n_args);
        return inst;
    }

    Instruction mul(int n_args) {
        Instruction inst(InstructionType::MUL);
        inst.value = make_value(ValueType::INT, n_args);
        return inst;
    }

    Instruction div() {
        return Instruction(InstructionType::DIV);
    }

    Instruction store(std::shared_ptr<AST> ast) {
        assert(ast->type == AST_SYMBOL);
        return Instruction(
            InstructionType::STORE, 
            make_value(ValueType::STRING, ast->string_value));
    }
}

void print_value(Value &v) {
    switch (v.type) {
        case ValueType::STRING:
        case ValueType::SYMBOL:
            std::cout << std::any_cast<std::string>(v.value) << std::endl;
            break;
        case ValueType::INT:
            std::cout << std::any_cast<int>(v.value) << std::endl;
            break;
        case ValueType::FLOAT:
            std::cout << std::any_cast<double>(v.value) << std::endl;
            break;    
        case ValueType::BOOL:
            std::cout << std::any_cast<bool>(v.value) << std::endl;
            break;
    }
}

void print_instructions(std::vector<Instruction> &instructions) {
    for (int i = 0; i < instructions.size(); i++) {
        auto& inst = instructions[i];

        std::cout << i << "\t";

        switch (inst.inst_type) {
            case InstructionType::PUSH:
                std::cout << "PUSH ";
                print_value(inst.value.value()); 
                break;
            case InstructionType::ADD:
                std::cout << "ADD ";
                print_value(inst.value.value()); 
                break;
            case InstructionType::SUB:
                std::cout << "SUB ";
                print_value(inst.value.value()); 
                break;
            case InstructionType::MUL:
                std::cout << "MUL ";
                print_value(inst.value.value()); 
                break;
            case InstructionType::DIV:
                std::cout << "DIV " << std::endl;
                break;
            case InstructionType::STORE:
                std::cout << "STORE ";
                print_value(inst.value.value()); 
                break;
            default:
                std::cout << "unknown instruction type" << std::endl;
                break;
        }
    }
}

void emit_expr(
    std::shared_ptr<AST> root, 
    std::vector<Instruction> &instructions) 
{

    for (auto& child : root->children) {

        if (child->type == AST_EXPR)
            emit_expr(child, instructions);
        else
            instructions.push_back(bytecode::push(child));
    }

    // Now add the operator:
    if (root->string_value == "+")
        instructions.push_back(bytecode::add(root->children.size()));
    else if (root->string_value == "-")
        instructions.push_back(bytecode::sub(root->children.size()));
    else if (root->string_value == "*")
        instructions.push_back(bytecode::mul(root->children.size()));
    else if (root->string_value == "/")
        instructions.push_back(bytecode::div());
    else {
        std::cout << "UNSUPPORTED OPERATOR :)" << std::endl;
    }
}

void emit_def(
    std::shared_ptr<AST> root, 
    std::vector<Instruction> &instructions) 
{
    auto& left = root->children[0];
    auto& right = root->children[1];

    if (right->type == AST_EXPR)
        emit_expr(right, instructions);
    else
        instructions.push_back(bytecode::push(right));
    
    // First child is always the symbol
    instructions.push_back(bytecode::store(left));
}


std::vector<Instruction> gen_bytecode(std::shared_ptr<AST> root) {
    std::vector<Instruction> instructions;

    for (auto& child : root->children) {
        // We'll keep it simple for now
        if (child->string_value == "def") {
            emit_def(child, instructions);
        }

        else {
            emit_expr(child, instructions);
        }
    }

    return instructions;
}

