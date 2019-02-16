﻿#include "autogen/asn_compiler.hpp"
#include "fast_ber/compiler/CompilerTypes.hpp"
#include "fast_ber/compiler/Dependencies.hpp"
#include "fast_ber/compiler/ReorderAssignments.hpp"

#include <unordered_map>

std::string strip_path(const std::string& path)
{
    std::size_t found = path.find_last_of("/\\");
    if (found == std::string::npos)
    {
        return path;
    }
    return path.substr(found + 1);
}

std::string create_assignment(const Assignment& assignment, TaggingMode tagging_mode)
{
    if (assignment.value) // Value assignment
    {
        std::string result;
        result += fully_tagged_type(assignment.type, tagging_mode) + " " + assignment.name + " = ObjectIdentifier{";
        if (absl::holds_alternative<std::vector<ObjectIdComponentValue>>(assignment.value->value_selection))
        {
            const std::vector<ObjectIdComponentValue>& object_id_component =
                absl::get<std::vector<ObjectIdComponentValue>>(assignment.value->value_selection);

            for (size_t i = 0; i < object_id_component.size(); i++)
            {
                if (object_id_component[i].value)
                {
                    result += std::to_string(*object_id_component[i].value);
                }
                else
                {
                    result += std::to_string(0);
                }

                if (i < object_id_component.size() - 1)
                {
                    result += ", ";
                }
            }
        }
        result += "};\n\n";
        return result;
    }
    else // Type assignment
    {
        if (absl::holds_alternative<BuiltinType>(assignment.type) &&
            absl::holds_alternative<SequenceType>(absl::get<BuiltinType>(assignment.type)))
        {
            const SequenceType& sequence = absl::get<SequenceType>(absl::get<BuiltinType>(assignment.type));
            return "struct " + assignment.name + to_string(sequence);
        }
        else if (absl::holds_alternative<BuiltinType>(assignment.type) &&
                 absl::holds_alternative<SetType>(absl::get<BuiltinType>(assignment.type)))
        {
            const SetType& set = absl::get<SetType>(absl::get<BuiltinType>(assignment.type));
            return "struct " + assignment.name + to_string(set);
        }
        else if (absl::holds_alternative<BuiltinType>(assignment.type) &&
                 absl::holds_alternative<EnumeratedType>(absl::get<BuiltinType>(assignment.type)))
        {
            const EnumeratedType& enumerated = absl::get<EnumeratedType>(absl::get<BuiltinType>(assignment.type));
            return "enum class " + assignment.name + to_string(enumerated);
        }
        else
        {
            return "using " + assignment.name + " = " + fully_tagged_type(assignment.type, tagging_mode) + ";\n\n";
        }
    }
}

std::string create_identifier_functions(const Assignment& assignment, const std::string& module_name)
{
    if (absl::holds_alternative<BuiltinType>(assignment.type) &&
        absl::holds_alternative<SequenceType>(absl::get<BuiltinType>(assignment.type)))
    {
        std::string       res;
        const std::string tags_class = assignment.name + "Tags";

        res += "constexpr inline ExplicitIdentifier<UniversalTag::sequence> identifier(const fast_ber::" + module_name +
               "::" + assignment.name + "*) noexcept";
        res += "\n{\n";
        res += "    return {};\n";
        res += "}\n\n";
        return res;
    }
    else if (absl::holds_alternative<BuiltinType>(assignment.type) &&
             absl::holds_alternative<SetType>(absl::get<BuiltinType>(assignment.type)))
    {
        std::string       res;
        const std::string tags_class = assignment.name + "Tags";

        res += "constexpr inline ExplicitIdentifier<UniversalTag::set> identifier(const fast_ber::" + module_name +
               "::" + assignment.name + "*) noexcept";
        res += "\n{\n";
        res += "    return {};\n";
        res += "}\n\n";
        return res;
    }
    else
    {
        return "";
    }
}

std::string create_encode_functions(const Assignment& assignment, const std::string& module_name,
                                    TaggingMode tagging_mode)
{
    if (absl::holds_alternative<BuiltinType>(assignment.type) &&
        absl::holds_alternative<SequenceType>(absl::get<BuiltinType>(assignment.type)))
    {
        std::string         res;
        const SequenceType& sequence   = absl::get<SequenceType>(absl::get<BuiltinType>(assignment.type));
        const std::string   tags_class = module_name + "_" + assignment.name + "Tags";

        res += "namespace " + tags_class + " {\n";
        int tag_counter = 0;
        for (const ComponentType& component : sequence.components)
        {
            res += "static const auto " + component.named_type.name + " = ";
            if (absl::holds_alternative<BuiltinType>(component.named_type.type) &&
                (absl::holds_alternative<PrefixedType>(absl::get<BuiltinType>(component.named_type.type))))
            {
                res += universal_tag(component.named_type.type, tagging_mode).tag;
            }
            else
            {
                res += "ImplicitIdentifier<Class::context_specific, " + std::to_string(tag_counter++) + ">";
            }
            res += "{};\n";
        }
        res += "}\n\n";

        res += "template <typename ID = ExplicitIdentifier<UniversalTag::sequence>>\n";
        res += "inline EncodeResult encode(absl::Span<uint8_t> output, const " + module_name + "::" + assignment.name +
               "& input, const ID& id = ID{}) noexcept\n{\n";
        res += "    return encode_sequence_combine(output, id";
        for (const ComponentType& component : sequence.components)
        {
            res += ",\n                          input." + component.named_type.name + ", " + tags_class +
                   "::" + component.named_type.name;
        }
        res += ");\n}\n\n";
        return res;
    }
    else if (absl::holds_alternative<BuiltinType>(assignment.type) &&
             absl::holds_alternative<SetType>(absl::get<BuiltinType>(assignment.type)))
    {
        std::string       res;
        const SetType&    set        = absl::get<SetType>(absl::get<BuiltinType>(assignment.type));
        const std::string tags_class = module_name + "_" + assignment.name + "Tags";

        res += "namespace " + tags_class + " {\n";
        int tag_counter = 0;
        for (const ComponentType& component : set.components)
        {
            res += "static const auto " + component.named_type.name + " = ";
            if (absl::holds_alternative<BuiltinType>(component.named_type.type) &&
                (absl::holds_alternative<PrefixedType>(absl::get<BuiltinType>(component.named_type.type))))
            {
                res += universal_tag(component.named_type.type, tagging_mode).tag;
            }
            else
            {
                res += "ImplicitIdentifier<Class::context_specific, " + std::to_string(tag_counter++) + ">";
            }
            res += "{};\n";
        }
        res += "}\n\n";

        res += "template <typename ID = ExplicitIdentifier<UniversalTag::set>>\n";
        res += "inline EncodeResult encode(absl::Span<uint8_t> output, const " + module_name + "::" + assignment.name +
               "& input, const ID& id = ID{}) noexcept\n{\n";
        res += "    return encode_set_combine(output, id";
        for (const ComponentType& component : set.components)
        {
            res += ",\n                          input." + component.named_type.name + ", " + tags_class +
                   "::" + component.named_type.name;
        }
        res += ");\n}\n\n";
        return res;
    }
    else
    {
        return "";
    }
}

std::string create_decode_functions(const Assignment& assignment, const std::string& module_name)
{
    if (absl::holds_alternative<BuiltinType>(assignment.type) &&
        absl::holds_alternative<SequenceType>(absl::get<BuiltinType>(assignment.type)))
    {
        const SequenceType& sequence = absl::get<SequenceType>(absl::get<BuiltinType>(assignment.type));
        std::string         res;
        const std::string   tags_class = module_name + "_" + assignment.name + "Tags";

        res += "template <typename ID = ExplicitIdentifier<UniversalTag::sequence>>\n";
        res += "inline DecodeResult decode(const BerView& input, " + module_name + "::" + assignment.name +
               "& output, const ID& id = ID{}) noexcept\n{\n";
        res += "    return decode_sequence_combine(input, \"" + module_name + "::" + assignment.name + "\", id";
        for (const ComponentType& component : sequence.components)
        {
            res += ",\n                          output." + component.named_type.name + ", " + tags_class +
                   "::" + component.named_type.name;
        }
        res += ");\n}\n\n";

        res += "template <typename ID = ExplicitIdentifier<UniversalTag::sequence>>\n";
        res += "inline DecodeResult decode(BerViewIterator& input, " + module_name + "::" + assignment.name +
               "& output, const ID& id = ID{}) noexcept\n{\n";
        res += "    DecodeResult result = decode(*input, output, id);\n";
        res += "    ++input;\n";
        res += "    return result;\n";
        res += "}\n\n";
        return res;
    }
    else if (absl::holds_alternative<BuiltinType>(assignment.type) &&
             absl::holds_alternative<SetType>(absl::get<BuiltinType>(assignment.type)))
    {
        const SetType&    set = absl::get<SetType>(absl::get<BuiltinType>(assignment.type));
        std::string       res;
        const std::string tags_class = module_name + "_" + assignment.name + "Tags";

        res += "template <typename ID = ExplicitIdentifier<UniversalTag::set>>\n";
        res += "inline DecodeResult decode(const BerView& input, " + module_name + "::" + assignment.name +
               "& output, const ID& id = ID{}) noexcept\n{\n";
        res += "    return decode_set_combine(input, \"" + module_name + "::" + assignment.name + "\", id";
        for (const ComponentType& component : set.components)
        {
            res += ",\n                          output." + component.named_type.name + ", " + tags_class +
                   "::" + component.named_type.name;
        }
        res += ");\n}\n\n";

        res += "template <typename ID = ExplicitIdentifier<UniversalTag::set>>\n";
        res += "inline DecodeResult decode(BerViewIterator& input, " + module_name + "::" + assignment.name +
               "& output, const ID& id = ID{}) noexcept\n{\n";
        res += "    DecodeResult result = decode(*input, output, id);\n";
        res += "    ++input;\n";
        res += "    return result;\n";
        res += "}\n\n";
        return res;
    }
    else
    {
        return "";
    }
}

std::string create_helper_functions(const Assignment& assignment)
{
    if (absl::holds_alternative<BuiltinType>(assignment.type) &&
        absl::holds_alternative<SequenceType>(absl::get<BuiltinType>(assignment.type)))
    {
        const SequenceType& sequence = absl::get<SequenceType>(absl::get<BuiltinType>(assignment.type));
        std::string         res;
        const std::string   tags_class = assignment.name + "Tags";

        res += "bool operator==(";
        res += "const " + assignment.name + "& lhs, ";
        res += "const " + assignment.name + "& rhs)\n";
        res += "{\n";
        res += "    return true";
        for (const ComponentType& component : sequence.components)
        {
            res += " &&\n      lhs." + component.named_type.name + " == ";
            res += "rhs." + component.named_type.name;
        }
        res += ";\n}\n\n";

        res += "bool operator!=(";
        res += "const " + assignment.name + "& lhs, ";
        res += "const " + assignment.name + "& rhs)\n";
        res += "{\n";
        res += "    return !(lhs == rhs);\n}\n\n";

        return res;
    }
    else if (absl::holds_alternative<BuiltinType>(assignment.type) &&
             absl::holds_alternative<SetType>(absl::get<BuiltinType>(assignment.type)))
    {
        const SetType&    set = absl::get<SetType>(absl::get<BuiltinType>(assignment.type));
        std::string       res;
        const std::string tags_class = assignment.name + "Tags";

        res += "bool operator==(";
        res += "const " + assignment.name + "& lhs, ";
        res += "const " + assignment.name + "& rhs)\n";
        res += "{\n";
        res += "    return true";
        for (const ComponentType& component : set.components)
        {
            res += " &&\n      lhs." + component.named_type.name + " == ";
            res += "rhs." + component.named_type.name;
        }
        res += ";\n}\n\n";

        res += "bool operator!=(";
        res += "const " + assignment.name + "& lhs, ";
        res += "const " + assignment.name + "& rhs)\n";
        res += "{\n";
        res += "    return !(lhs == rhs);\n}\n\n";

        return res;
    }
    else
    {
        return "";
    }
}

std::string create_include(const std::string& path) { return "#include \"" + path + "\"\n"; }

std::string add_namespace(const std::string& name, const std::string& enclosed)
{
    std::string output;

    output += "namespace " + name + " {\n";
    output += enclosed;
    output += "} // End namespace " + name + "\n";

    return output;
}

std::string create_body(const Module& module)
{
    std::string output;
    output += "\n";

    for (const Import& import : module.imports)
    {
        for (const auto& import_name : import.imports)
        {
            output += "using " + import_name + " = " + import.module_reference + "::" + import_name + ";\n";
        }
        if (import.imports.size() > 0)
        {
            output += "\n";
        }
    }

    for (const Assignment& assignment : module.assignments)
    {
        output += create_assignment(assignment, module.tagging_default);
    }

    return output;
}

std::string create_detail_body(const Asn1Tree& tree)
{
    std::string output;
    output += "/* Functionality provided for Encoding and Decoding BER */\n\n";

    std::string body;

    for (const auto& module : tree.modules)
    {
        // Inside namespace due to argument dependant lookup rules
        std::string ids;
        for (const Assignment& assignment : module.assignments)
        {
            ids += create_identifier_functions(assignment, module.module_reference);
        }
        body += add_namespace(module.module_reference, ids);

        for (const Assignment& assignment : module.assignments)
        {
            body += create_encode_functions(assignment, module.module_reference, module.tagging_default);
            body += create_decode_functions(assignment, module.module_reference) + "\n";
        }

        std::string helpers;
        for (const Assignment& assignment : module.assignments)
        {
            helpers += create_helper_functions(assignment);
        }

        body += add_namespace(module.module_reference, helpers);
    }
    output += add_namespace("fast_ber", body) + "\n\n";
    return output;
}

std::string create_output_file(const Asn1Tree& tree, const std::string detail_filename)
{
    std::string output;
    output += "#pragma once\n\n";
    output += create_include("fast_ber/ber_types/All.hpp");
    output += "\n";

    std::string definitions;
    for (const auto& module : tree.modules)
    {
        definitions += add_namespace(module.module_reference, create_body(module));
    }

    output += add_namespace("fast_ber", definitions) + '\n';
    output += create_include(strip_path(detail_filename)) + '\n';
    return output;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: INPUT.asn... OUTPUT_NAME\n";
        return -1;
    }

    try
    {
        Context context;
        for (int i = 1; i < argc - 1; i++)
        {
            std::string   input_filename = argv[i];
            std::ifstream input_file(input_filename);
            if (!input_file.good())
            {
                std::cout << "Could not open input file: " << input_filename << "\n";
                return -1;
            }

            std::string buffer(std::istreambuf_iterator<char>(input_file), {});
            context.cursor                  = buffer.c_str();
            context.location.begin.filename = &input_filename;
            context.location.end.filename   = &input_filename;

            yy::asn1_parser parser(context);

            const auto res = parser.parse();
            if (res)
            {
                return -1;
            }
        }

        const std::string& output_filename = std::string(argv[argc - 1]) + ".hpp";
        const std::string& detail_filame   = std::string(argv[argc - 1]) + ".detail.hpp";

        std::ofstream output_file(output_filename);
        std::ofstream detail_output_file(detail_filame);
        if (!output_file.good())
        {
            std::cout << "Could not create output file: " + output_filename + "\n";
            return -1;
        }
        if (!detail_output_file.good())
        {
            std::cout << "Could not create output file: " + detail_filame + "\n";
            return -1;
        }

        context.asn1_tree.modules = reorder_modules(context.asn1_tree.modules);

        for (auto& module : context.asn1_tree.modules)
        {
            module.assignments = reorder_assignments(module.assignments, module.imports);
        }

        output_file << create_output_file(context.asn1_tree, detail_filame);
        detail_output_file << create_detail_body(context.asn1_tree);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cout << "Compilation error: " << e.what() << "\n";
        return -1;
    }
}
