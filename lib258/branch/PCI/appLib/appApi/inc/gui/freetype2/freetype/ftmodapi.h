/***************************************************************************/
/*                                                                         */
/*  ftmodapi.h                                                             */
/*                                                                         */
/*    FreeType modules public interface (specification).                   */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2006, 2008 by                         */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTMODAPI_H__
#define __FTMODAPI_H__


#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    module_management                                                  */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Module Management                                                  */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    How to add, upgrade, and remove modules from FreeType.             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The definitions below are used to manage modules within FreeType.  */
  /*    Modules can be added, upgraded, and removed at runtime.            */
  /*                                                                       */
  /*************************************************************************/


  /* module bit flags */
#define FT_MODULE_FONT_DRIVER         1  /* this module is a font driver  */
#define FT_MODULE_RENDERER            2  /* this module is a renderer     */
#define FT_MODULE_HINTER              4  /* this module is a glyph hinter */
#define FT_MODULE_STYLER              8  /* this module is a styler       */

#define FT_MODULE_DRIVER_SCALABLE     0x100   /* the driver supports      */
                                              /* scalable fonts           */
#define FT_MODULE_DRIVER_NO_OUTLINES  0x200   /* the driver does not      */
                                              /* support vector outlines  */
#define FT_MODULE_DRIVER_HAS_HINTER   0x400   /* the driver provides its  */
                                              /* own hinter               */


  /* deprecated values */
#define ft_module_font_driver         FT_MODULE_FONT_DRIVER
#define ft_module_renderer            FT_MODULE_RENDERER
#define ft_module_hinter              FT_MODULE_HINTER
#define ft_module_styler              FT_MODULE_STYLER

#define ft_module_driver_scalable     FT_MODULE_DRIVER_SCALABLE
#define ft_module_driver_no_outlines  FT_MODULE_DRIVER_NO_OUTLINES
#define ft_module_driver_has_hinter   FT_MODULE_DRIVER_HAS_HINTER


  typedef FT_Pointer  FT_Module_Interface;


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Module_Constructor                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to initialize (not create) a new module object.    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    module :: The module to initialize.                                */
  /*                                                                       */
  typedef FT_Error
  (*FT_Module_Constructor)( FT_Module  module );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Module_Destructor                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to finalize (not destroy) a given module object.   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    module :: The module to finalize.                                  */
  /*                                                                       */
  typedef void
  (*FT_Module_Destructor)( FT_Module  module );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Module_Requester                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to query a given module for a specific interface.  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    module :: The module to finalize.                                  */
  /*                                                                       */
  /*    name ::   The name of the interface in the module.                 */
  /*                                                                       */
  typedef FT_Module_Interface
  (*FT_Module_Requester)( FT_Module    module,
                          const char*  name );


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Module_Class                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The module class descriptor.                                       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    module_flags    :: Bit flags describing the module.                */
  /*                                                                       */
  /*    module_size     :: The size of one module object/instance in       */
  /*                       bytes.                                          */
  /*                                                                       */
 }%    `à            Ù    àú      ¨       “    P‚     V       Û     „     {       ñ    ğ›                ğÍ     @      ?    àP     <       w    €           â    @Ê      /       ¹     Ö      6       Š"    @Í     %       {    `Q     M       ½	    `ˆ            b    °Q     >           €            (     <     Œ      O    €j     O       h    àH     J       7     ©     C       é    ¤               ³     6       
    °°     £       ×     ÀÅ      Ü       ½    ğ;     ©       _%    æ     Õ      –    °µ            !    Ğg            E    @È      ì       r
    àô      £       ‘    0¤     `            Ê      /       @    0>     ª      —     PÃ     ä       q     †     ®       ¢    À            `    0Ï     Ê       ô    @n           v    0É      Ø       —     ğÃ      Ê       Y    pˆ            ’    0`     /      ü    Ù      ½      "    Ë     z       &
    py     ¸      ñ     °     ˆ       -    Ğ     ¨        __gmon_start__ _fini __cxa_finalize _Jv_RegisterClasses dmap_marshal_STRING__STRING g_return_if_fail_warning g_value_peek_pointer g_value_take_string dmap_marshal_VOID__ULONG_FLOAT dmap_marshal_STRING__ULONG_ULONG dmap_marshal_ULONG__VOID g_value_set_ulong dmap_marshal_VOID__STRING_STRING dmap_marshal_VOID__STRING_BOOLEAN dmap_marshal_BOOLEAN__STRING g_value_set_boolean dmap_marshal_VOID__STRING_POINTER_POINTER_POINTER_BOOLEAN dacp_repeat_state_get_type g_enum_register_static dacp_play_state_get_type dmap_connection_state_get_type dmap_mdns_browser_service_type_get_type dmap_mdns_browser_transport_protocol_get_type dmap_mdns_browser_error_get_type dmap_mdns_publisher_error_get_type dmap_media_kind_get_type dmap_type_get_type g_type_class_peek_parent g_type_check_class_cast dmap_connection_get_type g_value_get_int dmap_record_factory_create g_object_set g_value_get_char g_value_get_string g_strdup daap_connection_get_type g_once_init_enter g_intern_static_string g_type_register_static_simple g_once_init_leave daap_connection_new g_object_new g_param_spec_string g_object_interface_install_property g_param_spec_enum g_param_spec_int64 g_param_spec_int g_param_spec_uint64 g_param_spec_boolean daap_record_get_type g_type_register_static daap_record_itunes_compat g_type_interface_peek daap_record_read daap_record_cmp_by_album dmap_db_lookup_by_id g_type_check_instance_cast g_object_get g_strcmp0 g_object_unref g_assertion_message_expr daap_share_get_desired_port daap_share_get_type_of_service dmap_structure_add daap_share_server_info g_log _dmap_share_get_auth_method _dmap_share_message_set_from_dmap_structure dmap_structure_destroy daap_share_message_add_standard_headers soup_message_headers_append dmap_share_get_type g_type_class_add_private strchr g_str_equal g_str_hash g_hash_table_new g_hash_table_lookup _dmap_share_build_filter dmap_db_apply_filter g_ascii_strcasecmp g_hash_table_foreach g_hash_table_size g_hash_table_get_keys g_list_sort g_list_foreach dmap_share_free_filter g_hash_table_destroy g_hash_table_insert g_type_name strtoul soup_message_headers_get strtoll g_strdup_printf soup_message_set_status g_malloc g_input_stream_get_type g_error_free g_seekable_get_type g_seekable_seek soup_message_body_set_accumulate soup_message_headers_set_encoding soup_message_headers_set_content_length dmap_write_next_chunk g_signal_connect_data dmap_chunked_message_finished strcmp dmap_gst_input_stream_new soup_message_get_http_version g_input_stream_close _dmap_share_client_requested daap_share_get_type g_type_instance_get_private daap_share_new g_object_ref _dmap_share_server_start _dmap_share_publish_start dacp_connection_get_type dacp_connection_new dacp_share_get_type_of_service g_signal_emit connection_handler_cb dmap_structure_find_item g_value_get_int64 g_hash_table_iter_init g_hash_table_iter_next dacp_player_now_playing_record g_slist_remove g_string_new g_get_host_name g_string_printf g_string_ascii_up g_string_append_len g_string_free mdns_remote_removed g_hash_table_remove mdns_remote_added dacp_share_ctrl_int dacp_share_login g_object_class_install_property g_param_spec_object g_signal_new g_cclosure_marshal_VOID__STRING _dmap_share_login dacp_share_get_type _dmap_share_session_id_validate g_strsplit g_strfreev strtod strtol g_slist_prepend g_signal_connect_object soup_server_pause_message dacp_player_now_playing_artwork gdk_pixbuf_new_from_file_at_scale gdk_pixbuf_save_to_buffer dacp_player_play_pause dacp_player_pause dacp_player_next_item dacp_player_prev_item g_hash_table_get_values dacp_player_cue_play g_hash_table_unref soup_message_set_response dacp_player_cue_clear g_list_sort_with_data g_slist_free g_value_dup_string dacp_player_get_type g_value_dup_object g_value_set_string g_value_set_object g_hash_table_new_full dacp_share_new dacp_share_start_lookup dmap_mdns_browser_new dmap_mdns_browser_start dacp_share_stop_lookup g_hash_table_foreach_remove dmap_mdns_browser_stop dacp_share_player_updated soup_server_unpause_message dacp_share_pair dmap_connection_setup g_strnfill g_strlcpy g_compute_checksum_for_data dmap_connection_get g_param_spec_ulong g_type_interface_add_prerequisite g_malloc0 soup_message_get_uri soup_uri_to_string strstr g_idle_add dmap_structure_parse g_source_remove inflateInit2_ g_realloc inflate inflateEnd g_slist_length dmap_structure_find_node g_slist_nth_data g_list_prepend g_list_reverse g_slist_sort g_direct_hash g_direct_equal dmap_db_add g_value_get_double g_param_spec_pointer g_param_spec_uint g_param_spec_double soup_auth_get_type soup_message_get_type soup_session_get_type g_cclosure_marshal_VOID__VOID g_thread_create soup_auth_authenticate soup_session_pause_message g_type_check_instance_is_a g_value_set_pointer g_value_set_uint g_value_set_int g_value_set_double dmap_db_get_type g_value_get_pointer dmap_record_factory_get_type g_value_get_uint soup_session_abort soup_uri_free dmap_connection_is_connected g_signal_handlers_disconnect_matched dmap_connection_authenticate_message soup_session_unpause_message soup_session_async_new soup_uri_new _SOUP_URI_SCHEME_HTTP soup_uri_set_scheme soup_uri_set_host soup_uri_set_port dmap_connection_connect dmap_connection_disconnect dmap_connection_get_headers g_ascii_strncasecmp floorf dmap_hash_generate soup_message_headers_new __stack_chk_fail dmap_connection_build_message soup_uri_new_with_base _SOUP_METHOD_GET soup_message_new_from_uri soup_session_queue_message dmap_connection_get_playlists dmap_container_db_get_type dmap_container_db_add dmap_container_db_lookup_by_id dmap_container_db_foreach dmap_container_db_count dmap_container_record_get_type dmap_container_record_get_id dmap_container_record_add_entry dmap_container_record_get_entry_count dmap_container_record_get_entries strrchr g_object_class_find_property g_value_init g_object_get_property g_type_check_value_holds g_value_type_transformable g_value_transform g_value_get_long g_value_unset g_value_get_boolean g_value_reset dmap_db_lookup_id_by_location dmap_db_foreach dmap_db_add_with_id dmap_db_add_path dmap_db_count _dmap_db_strsplit_using_quotes strlen g_strconcat memcpy memset __sprintf_chk dmap_record_get_type dmap_record_to_blob dmap_record_set_from_blob _dmap_share_ctrl_int dmap_structure_serialize soup_message_body_append soup_message_body_complete g_slist_append dmap_structure_get_size g_hash_table_lookup_adapter _dmap_share_add_playlist_to_mlcl _dmap_share_soup_auth_filter g_str_has_prefix soup_auth_domain_basic_new soup_auth_domain_basic_set_auth_callback soup_server_add_auth_domain soup_server_add_handler soup_server_run_async _dmap_share_content_codes _dmap_share_logout _dmap_share_update _dmap_share_published _dmap_share_name_collision _dmap_share_databases soup_server_get_type g_strv_get_type g_param_spec_boxed soup_server_quit dmap_mdns_publisher_rename_at_port dmap_mdns_publisher_withdraw dmap_mdns_publisher_new soup_address_new_any soup_server_new soup_server_get_port dmap_mdns_publisher_publish g_value_dup_boxed _dmap_share_get_revision_number g_value_set_boxed _dmap_share_get_revision_number_from_query soup_client_context_get_host _dmap_share_session_id_create g_random_int _dmap_share_session_id_remove dmap_content_codes dmap_content_code_string_as_int32 _dmap_share_uri_is_local _dmap_share_parse_meta_str _dmap_share_parse_meta g_string_insert_c dmap_structure_increase_by_predicted_size g_str_has_suffix dmap_content_code_name g_print g_node_depth g_type_check_value g_strdup_value_contents dmap_content_code_dmap_type dmap_content_code_string g_byte_array_append g_byte_array_new g_node_traverse g_byte_array_free dmap_content_code_read_from_buffer g_node_new g_node_insert_before g_utf8_validate g_memdup g_value_set_int64 g_value_set_char g_strndup g_node_unlink g_node_destroy g_type_value_table_peek g_assertion_message strncpy dmap_structure_print g_input_stream_read g_byte_array_sized_new g_byte_array_unref dpap_connection_get_type dpap_connection_new dpap_record_get_type dpap_record_read dpap_share_get_desired_port dpap_share_get_type_of_service dpap_share_server_info dpap_share_message_add_standard_headers g_mapped_file_free g_file_new_for_uri g_file_get_path g_mapped_file_new g_mapped_file_get_contents g_mapped_file_get_length dpap_share_get_type dpap_share_new avahi_entry_group_reset avahi_client_errno avahi_strerror dmap_mdns_avahi_get_client avahi_glib_allocator avahi_set_allocator avahi_glib_poll_new avahi_glib_poll_get avahi_client_new dmap_mdns_avahi_set_entry_group avahi_entry_group_get_client avahi_string_list_get_pair avahi_address_snprint avahi_service_resolver_free avahi_service_resolver_get_client g_cclosure_marshal_VOID__POINTER g_signal_handlers_destroy getenv avahi_service_resolver_new dmap_mdns_browser_get_type g_slist_foreach avahi_service_browser_free avahi_client_free avahi_glib_poll_free dmap_mdns_browser_error_quark g_quark_from_static_string avahi_service_browser_new dcgettext g_set_error dmap_mdns_browser_get_services dmap_mdns_browser_get_service_type g_object_add_weak_pointer dmap_mdns_publisher_get_type avahi_entry_group_free dmap_mdns_publisher_error_quark avahi_string_list_new avahi_string_list_add avahi_entry_group_add_service_strlst avahi_string_list_free avahi_entry_group_commit avahi_entry_group_new g_strdupv g_io_error_quark dmap_gst_input_stream_new_buffer_cb g_mutex_lock g_get_current_time g_time_val_add gst_app_sink_get_type gst_app_sink_pull_buffer g_queue_get_length g_queue_push_tail gst_mini_object_unref g_cond_signal g_mutex_unlock g_cond_timed_wait dmap_gst_wav_input_stream_new dmap_gst_mp3_input_stream_new dmapd_input_stream_strdup_format_extension dmap_gst_input_stream_get_type g_type_add_interface_static g_queue_free g_queue_pop_head g_queue_new g_cond_new g_mutex_new gst_element_get_static_pad pads_compatible gst_pad_link gst_element_link_many dmap_gst_mp3_input_stream_get_type gst_element_set_state gst_object_get_type gst_object_unref gst_pipeline_new gst_element_factory_make gst_element_get_type gst_bin_get_type gst_bin_add_many gst_element_link gst_app_sink_set_max_buffers gst_app_sink_set_drop gst_element_get_state gst_element_link_filtered dmap_gst_wav_input_stream_get_type gst_caps_new_simple gst_pad_get_caps gst_caps_intersect gst_caps_is_empty gst_caps_unref libgdk_pixbuf-2.0.so.0 libavahi-common.so.3 libavahi-client.so.3 libavahi-glib.so.1 libgstapp-0.10.so.0 libgstreamer-0.10.so.0 libsoup-2.4.so.1 libgio-2.0.so.0 libgobject-2.0.so.0 libglib-2.0.so.0 libz.so.1 libpthread.so.0 libc.so.6 _edata __bss_start _end libdmapsharing-3.0.so.2 GLIBC_2.3.4 GLIBC_2.4 GLIBC_2.2.5                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            )         ti	   :)     ii   F)     ui	   P)      "            Š÷     "            ›÷      "             ÷     ("            ³÷     8"            º÷     @"            Ê÷     h"            Î÷     p"            à÷     €"            è÷     ˆ"            ù÷     ˜"             ø      "            ‰3     È"            ø     Ğ"             ø     à"            )ø     è"            Ï     ø"            4ø      ‘"            Mø     ‘"            aø     ‘"            rø     (‘"            ~ø     0‘"            ø     @‘"            —ø     H‘"            ªø     X‘"            ¸ø     `‘"            Òø     p‘"            çø     x‘"            î!     ˆ‘"            óø     ‘"            t     È‘"            ¨ú     Ğ‘"            ıø     à‘"            Ğú     è‘"            ù     ø‘"            øú      ’"            
ù     ’"             û     ’"            ù     (’"            Hû     0’"            ù     @’"            pû     H’"            ù     ˆ’"            ˜û     ’"            Ô      ’"            Èû     ¨’"            ù     ¸’"            øû     À’"            ù     è’"            (ü     ğ’"            "ù      “"            Pü     “"            .ù     H“"            pü     P“"            "ù     `“"            ˜ü     h“"            .ù     ¨“"            5ù     °“"            Kù     À“"            Qù     È“"            gù     Ø“"            mù     à“"            …ù     ğ“"            ù     ø“"            ¥ù     (”"            ­ù     0”"            ¼ù     @”"            Áù     H”"            Öù     X”"            áù     `”"            ñù     p”"            ÷ù     x”"            İù     ˆ”"            ú     ”"            ú      ”"            ú     ¨”"            k+     ¸”"            ,ú     À”"            Š;     Ğ”"            ;ú     Ø”"            ´     è”"            Mú     ğ”"            G+      •"            aú     •"            sú     H•"             Ò      P•"            Ò      ¨•"                 °•"            €     –"            ÀH     –"            ĞH     h–"            0J     p–"             J     È–"            €K     Ğ–"            K     (—"            `g     0—"            pg     ˆ—"            0h     —"            @h     è—"            ª*     ğ—"            ª*     ˜"            +     ˜"            +     (˜"            +     0˜"            )+     H˜"            h     P˜"            .+     h˜"            t     p˜"            3+     ˆ˜"            ‚     ˜"            8+     ¨˜"               