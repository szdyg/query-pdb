#include <cstring>
#include <spdlog/spdlog.h>
#include "pdb_helper.h"
#include "pdb_parser.h"

namespace {

// CodeView stores a value either inline as a 16 bit unsigned integer (when it is
// smaller than LF_NUMERIC), or as a numeric leaf: a 2 byte kind tag followed by a
// payload whose width and signedness are determined by that tag.
int64_t read_numeric_leaf(const char *data, PDB::CodeView::TPI::TypeRecordKind kind) {
    using PDB::CodeView::TPI::TypeRecordKind;

    if (kind < TypeRecordKind::LF_NUMERIC) {
        return *reinterpret_cast<const uint16_t *>(data);
    }

    const char *payload = data + sizeof(TypeRecordKind);
    switch (kind) {
        // note: LF_CHAR and LF_NUMERIC share the value 0x8000
        case TypeRecordKind::LF_CHAR:
            return *reinterpret_cast<const int8_t *>(payload);
        case TypeRecordKind::LF_SHORT:
            return *reinterpret_cast<const int16_t *>(payload);
        case TypeRecordKind::LF_USHORT:
            return *reinterpret_cast<const uint16_t *>(payload);
        case TypeRecordKind::LF_LONG:
            return *reinterpret_cast<const int32_t *>(payload);
        case TypeRecordKind::LF_ULONG:
            return *reinterpret_cast<const uint32_t *>(payload);
        case TypeRecordKind::LF_QUADWORD:
            return *reinterpret_cast<const int64_t *>(payload);
        case TypeRecordKind::LF_UQUADWORD:
            return static_cast<int64_t>(*reinterpret_cast<const uint64_t *>(payload));
        default:
            spdlog::warn("unsupported numeric leaf 0x{:04x}",
                         static_cast<unsigned int>(kind));
            return -1;
    }
}

}

pdb_parser::pdb_parser(const std::string &filename)
        : file_(MemoryMappedFile::Open(filename.c_str())) {}

std::map<std::string, int64_t> pdb_parser::get_symbols(const std::set<std::string> &names) const {
    return call_with_pdb_stream(get_symbols_impl, names);
}

std::map<std::string, std::map<std::string, field_info>>
pdb_parser::get_struct(const std::set<std::string> &names) const {
    return call_with_pdb_stream(get_struct_impl, names);
}

std::map<std::string, std::map<std::string, int64_t>>
pdb_parser::get_enum(const std::set<std::string> &names) const {
    return call_with_pdb_stream(get_enum_impl, names);
}

std::map<std::string, int64_t>
pdb_parser::get_symbols_impl(
        const PDB::RawFile &raw_file,
        const PDB::DBIStream &dbi_stream,
        const PDB::TPIStream &tpi_stream,
        const std::set<std::string> &names
) {
    const PDB::ImageSectionStream image_section_stream =
            dbi_stream.CreateImageSectionStream(raw_file);
    const PDB::ModuleInfoStream module_info_stream =
            dbi_stream.CreateModuleInfoStream(raw_file);
    const PDB::CoalescedMSFStream symbol_record_stream =
            dbi_stream.CreateSymbolRecordStream(raw_file);

    std::map<std::string, int64_t> result;

    // read public symbols
    const PDB::PublicSymbolStream public_symbol_stream =
            dbi_stream.CreatePublicSymbolStream(raw_file);
    {
        const PDB::ArrayView<PDB::HashRecord> hash_records = public_symbol_stream.GetRecords();

        for (const PDB::HashRecord &hash_record: hash_records) {
            const PDB::CodeView::DBI::Record *record = public_symbol_stream.GetRecord(
                    symbol_record_stream, hash_record);
            const uint32_t rva = image_section_stream.ConvertSectionOffsetToRVA(
                    record->data.S_PUB32.section,
                    record->data.S_PUB32.offset);
            if (rva == 0u) {
                // certain symbols (e.g. control-flow guard symbols)
                // don't have a valid RVA, ignore those
                continue;
            }

            auto name = record->data.S_PUB32.name;
            if (names.find(name) != names.end()) {
                result.insert({name, rva});
            }
        }
    }

    // read global symbols
    const PDB::GlobalSymbolStream global_symbol_stream =
            dbi_stream.CreateGlobalSymbolStream(raw_file);
    {
        const PDB::ArrayView<PDB::HashRecord> hash_records = global_symbol_stream.GetRecords();

        for (const PDB::HashRecord &hash_record: hash_records) {
            const PDB::CodeView::DBI::Record *record = global_symbol_stream.GetRecord(
                    symbol_record_stream, hash_record);

            const char *name = nullptr;
            uint32_t rva = 0u;

            if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_GDATA32) {
                name = record->data.S_GDATA32.name;
                rva = image_section_stream.ConvertSectionOffsetToRVA(
                        record->data.S_GDATA32.section,
                        record->data.S_GDATA32.offset);
            } else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_GTHREAD32) {
                name = record->data.S_GTHREAD32.name;
                rva = image_section_stream.ConvertSectionOffsetToRVA(
                        record->data.S_GTHREAD32.section,
                        record->data.S_GTHREAD32.offset);
            } else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_LDATA32) {
                name = record->data.S_LDATA32.name;
                rva = image_section_stream.ConvertSectionOffsetToRVA(
                        record->data.S_LDATA32.section,
                        record->data.S_LDATA32.offset);
            } else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_LTHREAD32) {
                name = record->data.S_LTHREAD32.name;
                rva = image_section_stream.ConvertSectionOffsetToRVA(
                        record->data.S_LTHREAD32.section,
                        record->data.S_LTHREAD32.offset);
            } else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_UDT) {
                name = record->data.S_UDT.name;
            } else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_UDT_ST) {
                name = record->data.S_UDT_ST.name;
            }

            if (rva == 0u) {
                // certain symbols (e.g. control-flow guard symbols)
                // don't have a valid RVA, ignore those
                continue;
            }
            if (names.find(name) != names.end()) {
                result.insert({name, rva});
            }
        }
    }

    // read module symbols
    const PDB::ArrayView<PDB::ModuleInfoStream::Module> modules = module_info_stream.GetModules();

    for (const PDB::ModuleInfoStream::Module &module: modules) {
        if (!module.HasSymbolStream()) {
            continue;
        }

        const PDB::ModuleSymbolStream module_symbol_stream =
                module.CreateSymbolStream(raw_file);
        module_symbol_stream.ForEachSymbol([&names, &result, &image_section_stream](
                const PDB::CodeView::DBI::Record *record) {
            const char *name = nullptr;
            uint32_t rva = 0u;
            if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_THUNK32) {
                if (record->data.S_THUNK32.thunk ==
                    PDB::CodeView::DBI::ThunkOrdinal::TrampolineIncremental) {
                    // we have never seen incremental linking thunks
                    // stored inside a S_THUNK32 symbol, but better be safe than sorry
                    name = "ILT";
                    rva = image_section_stream.ConvertSectionOffsetToRVA(
                            record->data.S_THUNK32.section,
                            record->data.S_THUNK32.offset);
                }
            } else if (record->header.kind ==
                       PDB::CodeView::DBI::SymbolRecordKind::S_TRAMPOLINE) {
                // incremental linking thunks are stored in the linker module
                name = "ILT";
                rva = image_section_stream.ConvertSectionOffsetToRVA(
                        record->data.S_TRAMPOLINE.thunkSection,
                        record->data.S_TRAMPOLINE.thunkOffset);
            } else if (record->header.kind ==
                       PDB::CodeView::DBI::SymbolRecordKind::S_BLOCK32) {
                // blocks never store a name and are only stored
                // for indicating whether other symbols are children of this block
            } else if (record->header.kind ==
                       PDB::CodeView::DBI::SymbolRecordKind::S_LABEL32) {
                // labels don't have a name
            } else if (record->header.kind ==
                       PDB::CodeView::DBI::SymbolRecordKind::S_LPROC32) {
                name = record->data.S_LPROC32.name;
                rva = image_section_stream.ConvertSectionOffsetToRVA(
                        record->data.S_LPROC32.section,
                        record->data.S_LPROC32.offset);
            } else if (record->header.kind ==
                       PDB::CodeView::DBI::SymbolRecordKind::S_GPROC32) {
                name = record->data.S_GPROC32.name;
                rva = image_section_stream.ConvertSectionOffsetToRVA(
                        record->data.S_GPROC32.section,
                        record->data.S_GPROC32.offset);
            } else if (record->header.kind ==
                       PDB::CodeView::DBI::SymbolRecordKind::S_LPROC32_ID) {
                name = record->data.S_LPROC32_ID.name;
                rva = image_section_stream.ConvertSectionOffsetToRVA(
                        record->data.S_LPROC32_ID.section,
                        record->data.S_LPROC32_ID.offset);
            } else if (record->header.kind ==
                       PDB::CodeView::DBI::SymbolRecordKind::S_GPROC32_ID) {
                name = record->data.S_GPROC32_ID.name;
                rva = image_section_stream.ConvertSectionOffsetToRVA(
                        record->data.S_GPROC32_ID.section,
                        record->data.S_GPROC32_ID.offset);
            } else if (record->header.kind ==
                       PDB::CodeView::DBI::SymbolRecordKind::S_REGREL32) {
                name = record->data.S_REGREL32.name;
                // You can only get the address while running the program by
                // checking the register value and adding the offset
            } else if (record->header.kind ==
                       PDB::CodeView::DBI::SymbolRecordKind::S_LDATA32) {
                name = record->data.S_LDATA32.name;
                rva = image_section_stream.ConvertSectionOffsetToRVA(
                        record->data.S_LDATA32.section,
                        record->data.S_LDATA32.offset);
            } else if (record->header.kind ==
                       PDB::CodeView::DBI::SymbolRecordKind::S_LTHREAD32) {
                name = record->data.S_LTHREAD32.name;
                rva = image_section_stream.ConvertSectionOffsetToRVA(
                        record->data.S_LTHREAD32.section,
                        record->data.S_LTHREAD32.offset);
            }

            if (rva == 0u) {
                // certain symbols (e.g. control-flow guard symbols)
                // don't have a valid RVA, ignore those
                return;
            }

            if (names.find(name) != names.end()) {
                result.insert({name, rva});
            }
        });
    }

    for (const std::string &name: names) {
        if (result.find(name) == result.end()) {
            result.insert({name, -1});
        }
    }

    return result;
}

std::map<std::string, std::map<std::string, field_info>>
pdb_parser::get_struct_impl(const PDB::RawFile &raw_file,
        const PDB::DBIStream &dbi_stream,
        const PDB::TPIStream &tpi_stream,
        const std::set<std::string> &names) {
    std::map<std::string, std::map<std::string, field_info>> result;

    // the tpi stream is walked lazily, the type table builds the index -> record
    // lookup that resolving field types needs
    const TypeTable type_table(tpi_stream);

    for (const auto &record: type_table.GetTypeRecords()) {
        if (record->header.kind == PDB::CodeView::TPI::TypeRecordKind::LF_STRUCTURE) {
            if (record->data.LF_CLASS.property.fwdref)
                continue;

            auto type_record = type_table.GetTypeRecord(record->data.LF_CLASS.field);
            if (!type_record)
                continue;

            auto leaf_name = GetLeafName(
                    record->data.LF_CLASS.data, record->data.LF_CLASS.lfEasy.kind);

            if (auto it = names.find(leaf_name); it != names.end()) {
                std::map<std::string, field_info> fields =
                        get_struct_single(type_table, type_record);
                result.insert({leaf_name, fields});
            }
        } else if (record->header.kind == PDB::CodeView::TPI::TypeRecordKind::LF_UNION) {
            if (record->data.LF_UNION.property.fwdref)
                continue;

            auto type_record = type_table.GetTypeRecord(record->data.LF_UNION.field);
            if (!type_record)
                continue;

            auto leaf_name = GetLeafName(
                    record->data.LF_UNION.data, static_cast<PDB::CodeView::TPI::TypeRecordKind>(0));

            if (auto it = names.find(leaf_name); it != names.end()) {
                std::map<std::string, field_info> fields =
                        get_struct_single(type_table, type_record);
                result.insert({leaf_name, fields});
            }
        }
    }

    return result;
}

std::map<std::string, field_info>
pdb_parser::get_struct_single(
        const TypeTable &type_table,
        const PDB::CodeView::TPI::Record *record
) {
    std::map<std::string, field_info> result;

    const PDB::CodeView::TPI::Record *referenced_type = nullptr;
    const PDB::CodeView::TPI::Record *modifier_record = nullptr;
    const char *leaf_name = nullptr;
    const char *type_name = nullptr;
    uint16_t offset = 0;

    auto maximum_size = record->header.size - sizeof(uint16_t);

    for (size_t i = 0; i < maximum_size;) {
        uint8_t pointer_level = 0;
        auto field_record = reinterpret_cast<const PDB::CodeView::TPI::FieldList *>(
                reinterpret_cast<const uint8_t *>(&record->data.LF_FIELD.list) + i);

        // Other kinds of records are not implemented
        if (field_record->kind != PDB::CodeView::TPI::TypeRecordKind::LF_BCLASS &&
            field_record->kind != PDB::CodeView::TPI::TypeRecordKind::LF_VFUNCTAB &&
            field_record->kind != PDB::CodeView::TPI::TypeRecordKind::LF_NESTTYPE &&
            field_record->kind != PDB::CodeView::TPI::TypeRecordKind::LF_ENUM &&
            field_record->kind != PDB::CodeView::TPI::TypeRecordKind::LF_MEMBER &&
            field_record->kind != PDB::CodeView::TPI::TypeRecordKind::LF_STMEMBER &&
            field_record->kind != PDB::CodeView::TPI::TypeRecordKind::LF_METHOD &&
            field_record->kind != PDB::CodeView::TPI::TypeRecordKind::LF_ONEMETHOD) {

            spdlog::warn("Unknown record kind {}",
                         static_cast<unsigned int>(field_record->kind));
        }

        if (field_record->kind == PDB::CodeView::TPI::TypeRecordKind::LF_MEMBER) {
            if (field_record->data.LF_MEMBER.lfEasy.kind <
                PDB::CodeView::TPI::TypeRecordKind::LF_NUMERIC)
                offset = *reinterpret_cast<const uint16_t *>(
                        &field_record->data.LF_MEMBER.offset[0]);
            else
                offset = *reinterpret_cast<const uint16_t *>(&field_record->data.LF_MEMBER.offset[
                        sizeof(PDB::CodeView::TPI::TypeRecordKind)]);

            leaf_name = GetLeafName(field_record->data.LF_MEMBER.offset,
                                    field_record->data.LF_MEMBER.lfEasy.kind);
            type_name = GetTypeName(type_table, field_record->data.LF_MEMBER.index,
                                    pointer_level, &referenced_type,
                                    &modifier_record);

                field_info info{};
                info.offset = offset;
                // GetTypeName returns the pointee's name and reports the
                // indirection separately, and it may return nullptr
                if (type_name) {
                    info.type = type_name;
                    info.type.append(pointer_level, '*');
                }
                if (referenced_type &&
                    referenced_type->header.kind ==
                    PDB::CodeView::TPI::TypeRecordKind::LF_BITFIELD) {
                    info.bitfield_offset = referenced_type->data.LF_BITFIELD.position;
                }
                result.insert({leaf_name, info});

        } else if (field_record->kind == PDB::CodeView::TPI::TypeRecordKind::LF_NESTTYPE) {
            leaf_name = &field_record->data.LF_NESTTYPE.name[0];
        } else if (field_record->kind == PDB::CodeView::TPI::TypeRecordKind::LF_STMEMBER) {
            leaf_name = &field_record->data.LF_STMEMBER.name[0];
        } else if (field_record->kind == PDB::CodeView::TPI::TypeRecordKind::LF_METHOD) {
            leaf_name = GetMethodName(field_record);
        } else if (field_record->kind == PDB::CodeView::TPI::TypeRecordKind::LF_ONEMETHOD) {
            leaf_name = GetMethodName(field_record);
        } else if (field_record->kind == PDB::CodeView::TPI::TypeRecordKind::LF_BCLASS) {
            leaf_name = GetLeafName(field_record->data.LF_BCLASS.offset,
                                    field_record->data.LF_BCLASS.lfEasy.kind);

            i += static_cast<size_t>(leaf_name - reinterpret_cast<const char *>(field_record));
            i = (i + (sizeof(uint32_t) - 1)) & (0 - sizeof(uint32_t));
            continue;
        } else if (field_record->kind == PDB::CodeView::TPI::TypeRecordKind::LF_VFUNCTAB) {
            i += sizeof(PDB::CodeView::TPI::FieldList::Data::LF_VFUNCTAB);
            i = (i + (sizeof(uint32_t) - 1)) & (0 - sizeof(uint32_t));
            continue;
        } else {
            break;
        }

        i += static_cast<size_t>(leaf_name - reinterpret_cast<const char *>(field_record));
        i += strnlen(leaf_name, maximum_size - i - 1) + 1;
        i = (i + (sizeof(uint32_t) - 1)) & (0 - sizeof(uint32_t));
    }

    return result;
}

std::map<std::string, std::map<std::string, int64_t>>
pdb_parser::get_enum_impl(
        const PDB::RawFile &raw_file,
        const PDB::DBIStream &dbi_stream,
        const PDB::TPIStream &tpi_stream,
        const std::set<std::string> &names
) {
    std::map<std::string, std::map<std::string, int64_t>> result;

    const TypeTable type_table(tpi_stream);

    for (const auto &record: type_table.GetTypeRecords()) {
        if (record->header.kind == PDB::CodeView::TPI::TypeRecordKind::LF_ENUM) {
            if (record->data.LF_ENUM.property.fwdref)
                continue;

            auto type_record = type_table.GetTypeRecord(record->data.LF_ENUM.field);
            if (!type_record)
                continue;

            auto leaf_name = record->data.LF_ENUM.name;
            if (auto it = names.find(leaf_name); it != names.end()) {
                std::map<std::string, int64_t> fields = get_enum_single(type_record);
                result.insert({leaf_name, fields});
            }
        }
    }

    return result;
}

std::map<std::string, int64_t>
pdb_parser::get_enum_single(const PDB::CodeView::TPI::Record *record) {
    std::map<std::string, int64_t> result;

    auto maximum_size = record->header.size - sizeof(uint16_t);

    for (size_t i = 0; i < maximum_size;) {
        auto field_record = reinterpret_cast<const PDB::CodeView::TPI::FieldList *>(
                reinterpret_cast<const uint8_t *>(&record->data.LF_FIELD.list) + i);

        // the value is stored before the name, and both its width and the offset
        // of the name that follows it depend on this kind tag
        auto kind = field_record->data.LF_ENUMERATE.lfEasy.kind;

        int64_t value = read_numeric_leaf(field_record->data.LF_ENUMERATE.value, kind);
        const char *leaf_name = GetLeafName(field_record->data.LF_ENUMERATE.value, kind);

        result.insert({leaf_name, value});

        i += static_cast<size_t>(leaf_name - reinterpret_cast<const char *>(field_record));
        i += strnlen(leaf_name, maximum_size - i - 1) + 1;
        i = (i + (sizeof(uint32_t) - 1)) & (0 - sizeof(uint32_t));
    }

    return result;
}
