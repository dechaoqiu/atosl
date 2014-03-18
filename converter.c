/*
 * =====================================================================================
 *
 *       Filename:  converter.c
 *
 *    Description:  Convert code to string or enum type
 *
 *        Version:  1.0
 *        Created:  21/02/13 08:54:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Reno Qiu
 *   Organization:
 *
 * =====================================================================================
 */

#include "converter.h"
/* Convert a DWARF attribute code into its string name.  */

const char* dwarf_attr_name (unsigned int attr)
{
    switch (attr)
    {
        case DW_AT_sibling:
            return "DW_AT_sibling";
        case DW_AT_location:
            return "DW_AT_location";
        case DW_AT_name:
            return "DW_AT_name";
        case DW_AT_ordering:
            return "DW_AT_ordering";
        case DW_AT_subscr_data:
            return "DW_AT_subscr_data";
        case DW_AT_byte_size:
            return "DW_AT_byte_size";
        case DW_AT_bit_offset:
            return "DW_AT_bit_offset";
        case DW_AT_bit_size:
            return "DW_AT_bit_size";
        case DW_AT_element_list:
            return "DW_AT_element_list";
        case DW_AT_stmt_list:
            return "DW_AT_stmt_list";
        case DW_AT_low_pc:
            return "DW_AT_low_pc";
        case DW_AT_high_pc:
            return "DW_AT_high_pc";
        case DW_AT_language:
            return "DW_AT_language";
        case DW_AT_member:
            return "DW_AT_member";
        case DW_AT_discr:
            return "DW_AT_discr";
        case DW_AT_discr_value:
            return "DW_AT_discr_value";
        case DW_AT_visibility:
            return "DW_AT_visibility";
        case DW_AT_import:
            return "DW_AT_import";
        case DW_AT_string_length:
            return "DW_AT_string_length";
        case DW_AT_common_reference:
            return "DW_AT_common_reference";
        case DW_AT_comp_dir:
            return "DW_AT_comp_dir";
        case DW_AT_const_value:
            return "DW_AT_const_value";
        case DW_AT_containing_type:
            return "DW_AT_containing_type";
        case DW_AT_default_value:
            return "DW_AT_default_value";
        case DW_AT_inline:
            return "DW_AT_inline";
        case DW_AT_is_optional:
            return "DW_AT_is_optional";
        case DW_AT_lower_bound:
            return "DW_AT_lower_bound";
        case DW_AT_producer:
            return "DW_AT_producer";
        case DW_AT_prototyped:
            return "DW_AT_prototyped";
        case DW_AT_return_addr:
            return "DW_AT_return_addr";
        case DW_AT_start_scope:
            return "DW_AT_start_scope";
        case DW_AT_stride_size:
            return "DW_AT_stride_size";
        case DW_AT_upper_bound:
            return "DW_AT_upper_bound";
        case DW_AT_abstract_origin:
            return "DW_AT_abstract_origin";
        case DW_AT_accessibility:
            return "DW_AT_accessibility";
        case DW_AT_address_class:
            return "DW_AT_address_class";
        case DW_AT_artificial:
            return "DW_AT_artificial";
        case DW_AT_base_types:
            return "DW_AT_base_types";
        case DW_AT_calling_convention:
            return "DW_AT_calling_convention";
        case DW_AT_count:
            return "DW_AT_count";
        case DW_AT_data_member_location:
            return "DW_AT_data_member_location";
        case DW_AT_decl_column:
            return "DW_AT_decl_column";
        case DW_AT_decl_file:
            return "DW_AT_decl_file";
        case DW_AT_decl_line:
            return "DW_AT_decl_line";
        case DW_AT_declaration:
            return "DW_AT_declaration";
        case DW_AT_discr_list:
            return "DW_AT_discr_list";
        case DW_AT_encoding:
            return "DW_AT_encoding";
        case DW_AT_external:
            return "DW_AT_external";
        case DW_AT_frame_base:
            return "DW_AT_frame_base";
        case DW_AT_friend:
            return "DW_AT_friend";
        case DW_AT_identifier_case:
            return "DW_AT_identifier_case";
        case DW_AT_macro_info:
            return "DW_AT_macro_info";
        case DW_AT_namelist_items:
            return "DW_AT_namelist_items";
        case DW_AT_priority:
            return "DW_AT_priority";
        case DW_AT_segment:
            return "DW_AT_segment";
        case DW_AT_specification:
            return "DW_AT_specification";
        case DW_AT_static_link:
            return "DW_AT_static_link";
        case DW_AT_type:
            return "DW_AT_type";
        case DW_AT_use_location:
            return "DW_AT_use_location";
        case DW_AT_variable_parameter:
            return "DW_AT_variable_parameter";
        case DW_AT_virtuality:
            return "DW_AT_virtuality";
        case DW_AT_vtable_elem_location:
            return "DW_AT_vtable_elem_location";
        case DW_AT_allocated:
            return "DW_AT_allocated";
        case DW_AT_associated:
            return "DW_AT_associated";
        case DW_AT_data_location:
            return "DW_AT_data_location";
        case DW_AT_stride:
            return "DW_AT_stride";
        case DW_AT_entry_pc:
            return "DW_AT_entry_pc";
        case DW_AT_use_UTF8:
            return "DW_AT_use_UTF8";
        case DW_AT_extension:
            return "DW_AT_extension";
        case DW_AT_ranges:
            return "DW_AT_ranges";
        case DW_AT_trampoline:
            return "DW_AT_trampoline";
        case DW_AT_call_column:
            return "DW_AT_call_column";
        case DW_AT_call_file:
            return "DW_AT_call_file";
        case DW_AT_call_line:
            return "DW_AT_call_line";
            //#ifdef MIPS
            //    case DW_AT_MIPS_fde:
            //      return "DW_AT_MIPS_fde";
            //    case DW_AT_MIPS_loop_begin:
            //      return "DW_AT_MIPS_loop_begin";
            //    case DW_AT_MIPS_tail_loop_begin:
            //      return "DW_AT_MIPS_tail_loop_begin";
            //    case DW_AT_MIPS_epilog_begin:
            //      return "DW_AT_MIPS_epilog_begin";
            //    case DW_AT_MIPS_loop_unroll_factor:
            //      return "DW_AT_MIPS_loop_unroll_factor";
            //    case DW_AT_MIPS_software_pipeline_depth:
            //      return "DW_AT_MIPS_software_pipeline_depth";
            //#endif
        case DW_AT_MIPS_linkage_name:
            return "DW_AT_MIPS_linkage_name";

        case DW_AT_sf_names:
            return "DW_AT_sf_names";
        case DW_AT_src_info:
            return "DW_AT_src_info";
        case DW_AT_mac_info:
            return "DW_AT_mac_info";
        case DW_AT_src_coords:
            return "DW_AT_src_coords";
        case DW_AT_body_begin:
            return "DW_AT_body_begin";
        case DW_AT_body_end:
            return "DW_AT_body_end";
        case DW_AT_GNU_vector:
            return "DW_AT_GNU_vector";
            //    /* APPLE LOCAL begin dwarf repository  */
            //    case DW_AT_APPLE_repository_file:
            //      return "DW_AT_APPLE_repository_file";
            //    case DW_AT_APPLE_repository_type:
            //      return "DW_AT_APPLE_repository_type";
            //    case DW_AT_APPLE_repository_name:
            //      return "DW_AT_APPLE_repository_name";
            //    case DW_AT_APPLE_repository_specification:
            //      return "DW_AT_APPLE_repository_specification";
            //    case DW_AT_APPLE_repository_import:
            //      return "DW_AT_APPLE_repository_import";
            //    case DW_AT_APPLE_repository_abstract_origin:
            //      return "DW_AT_APPLE_repository_abstract_origin";
            /* APPLE LOCAL end dwarf repository  */
        default:
            return "DW_AT_<unknown>";
    }
}



/* Convert a DWARF value form code into its string name.  */

const char * dwarf_form_name (unsigned form)
{
    switch (form)
    {
        case DW_FORM_addr:
            return "DW_FORM_addr";
        case DW_FORM_block2:
            return "DW_FORM_block2";
        case DW_FORM_block4:
            return "DW_FORM_block4";
        case DW_FORM_data2:
            return "DW_FORM_data2";
        case DW_FORM_data4:
            return "DW_FORM_data4";
        case DW_FORM_data8:
            return "DW_FORM_data8";
        case DW_FORM_string:
            return "DW_FORM_string";
        case DW_FORM_block:
            return "DW_FORM_block";
        case DW_FORM_block1:
            return "DW_FORM_block1";
        case DW_FORM_data1:
            return "DW_FORM_data1";
        case DW_FORM_flag:
            return "DW_FORM_flag";
        case DW_FORM_sdata:
            return "DW_FORM_sdata";
        case DW_FORM_strp:
            return "DW_FORM_strp";
        case DW_FORM_udata:
            return "DW_FORM_udata";
        case DW_FORM_ref_addr:
            return "DW_FORM_ref_addr";
        case DW_FORM_ref1:
            return "DW_FORM_ref1";
        case DW_FORM_ref2:
            return "DW_FORM_ref2";
        case DW_FORM_ref4:
            return "DW_FORM_ref4";
        case DW_FORM_ref8:
            return "DW_FORM_ref8";
        case DW_FORM_ref_udata:
            return "DW_FORM_ref_udata";
        case DW_FORM_indirect:
            return "DW_FORM_indirect";
            //case DW_FORM_APPLE_db_str:
            //  return "DW_FORM_APPLE_db_str";
        default:
            return "DW_FORM_<unknown>";
    }
}

/* Convert a DIE tag into its string name.  */

const char * dwarf_tag_name (unsigned tag)
{
    switch (tag)
    {
        case DW_TAG_padding:
            return "DW_TAG_padding";
        case DW_TAG_array_type:
            return "DW_TAG_array_type";
        case DW_TAG_class_type:
            return "DW_TAG_class_type";
        case DW_TAG_entry_point:
            return "DW_TAG_entry_point";
        case DW_TAG_enumeration_type:
            return "DW_TAG_enumeration_type";
        case DW_TAG_formal_parameter:
            return "DW_TAG_formal_parameter";
        case DW_TAG_imported_declaration:
            return "DW_TAG_imported_declaration";
        case DW_TAG_label:
            return "DW_TAG_label";
        case DW_TAG_lexical_block:
            return "DW_TAG_lexical_block";
        case DW_TAG_member:
            return "DW_TAG_member";
        case DW_TAG_pointer_type:
            return "DW_TAG_pointer_type";
        case DW_TAG_reference_type:
            return "DW_TAG_reference_type";
        case DW_TAG_compile_unit:
            return "DW_TAG_compile_unit";
        case DW_TAG_string_type:
            return "DW_TAG_string_type";
        case DW_TAG_structure_type:
            return "DW_TAG_structure_type";
        case DW_TAG_subroutine_type:
            return "DW_TAG_subroutine_type";
        case DW_TAG_typedef:
            return "DW_TAG_typedef";
        case DW_TAG_union_type:
            return "DW_TAG_union_type";
        case DW_TAG_unspecified_parameters:
            return "DW_TAG_unspecified_parameters";
        case DW_TAG_variant:
            return "DW_TAG_variant";
        case DW_TAG_common_block:
            return "DW_TAG_common_block";
        case DW_TAG_common_inclusion:
            return "DW_TAG_common_inclusion";
        case DW_TAG_inheritance:
            return "DW_TAG_inheritance";
        case DW_TAG_inlined_subroutine:
            return "DW_TAG_inlined_subroutine";
        case DW_TAG_module:
            return "DW_TAG_module";
        case DW_TAG_ptr_to_member_type:
            return "DW_TAG_ptr_to_member_type";
        case DW_TAG_set_type:
            return "DW_TAG_set_type";
        case DW_TAG_subrange_type:
            return "DW_TAG_subrange_type";
        case DW_TAG_with_stmt:
            return "DW_TAG_with_stmt";
        case DW_TAG_access_declaration:
            return "DW_TAG_access_declaration";
        case DW_TAG_base_type:
            return "DW_TAG_base_type";
        case DW_TAG_catch_block:
            return "DW_TAG_catch_block";
        case DW_TAG_const_type:
            return "DW_TAG_const_type";
        case DW_TAG_constant:
            return "DW_TAG_constant";
        case DW_TAG_enumerator:
            return "DW_TAG_enumerator";
        case DW_TAG_file_type:
            return "DW_TAG_file_type";
        case DW_TAG_friend:
            return "DW_TAG_friend";
        case DW_TAG_namelist:
            return "DW_TAG_namelist";
        case DW_TAG_namelist_item:
            return "DW_TAG_namelist_item";
        case DW_TAG_packed_type:
            return "DW_TAG_packed_type";
        case DW_TAG_subprogram:
            return "DW_TAG_subprogram";
        case DW_TAG_template_type_param:
            return "DW_TAG_template_type_param";
        case DW_TAG_template_value_param:
            return "DW_TAG_template_value_param";
        case DW_TAG_thrown_type:
            return "DW_TAG_thrown_type";
        case DW_TAG_try_block:
            return "DW_TAG_try_block";
        case DW_TAG_variant_part:
            return "DW_TAG_variant_part";
        case DW_TAG_variable:
            return "DW_TAG_variable";
        case DW_TAG_volatile_type:
            return "DW_TAG_volatile_type";
        case DW_TAG_dwarf_procedure:
            return "DW_TAG_dwarf_procedure";
        case DW_TAG_restrict_type:
            return "DW_TAG_restrict_type";
        case DW_TAG_interface_type:
            return "DW_TAG_interface_type";
        case DW_TAG_namespace:
            return "DW_TAG_namespace";
        case DW_TAG_imported_module:
            return "DW_TAG_imported_module";
        case DW_TAG_unspecified_type:
            return "DW_TAG_unspecified_type";
        case DW_TAG_partial_unit:
            return "DW_TAG_partial_unit";
        case DW_TAG_imported_unit:
            return "DW_TAG_imported_unit";
        case DW_TAG_MIPS_loop:
            return "DW_TAG_MIPS_loop";
        case DW_TAG_format_label:
            return "DW_TAG_format_label";
        case DW_TAG_function_template:
            return "DW_TAG_function_template";
        case DW_TAG_class_template:
            return "DW_TAG_class_template";
        default:
            return "DW_TAG_<unknown>";
    }
}

