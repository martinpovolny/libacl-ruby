#include <ruby.h>

#include <sys/types.h>
#include <sys/acl.h>

VALUE cACL, cEntry;

void ruby_acl_free(acl_t acl) {
  acl_free( acl );
}

#define Data_Get_Struct_ex(obj,type,sval) do {\
    Check_Type(obj, T_DATA); \
    sval = (type)DATA_PTR(obj);\
} while (0)

/* 
 * dump ACL as text
 *
 * call-seq:
 *   acl.to_text -> string
 *
 */
VALUE ruby_acl_to_text( VALUE self ) {
  acl_t acl;
  char *text;
  VALUE ret;
  
  Data_Get_Struct_ex( self, acl_t, acl );
  if ( NULL == (text = acl_to_text( acl, NULL ) ) ) {
    rb_sys_fail("acl_to_text returned error");
  }
  ret = rb_str_new2( text );
  acl_free( text );

  return ret;
}

/*
 * iterate over ACL entries
 *
 * call-seq:
 *    acl.each { |entry| }
 *
 */
VALUE ruby_acl_each(VALUE self) {
  int have;
  acl_t acl;
  acl_entry_t acl_entry;

  Data_Get_Struct_ex( self, acl_t, acl );
  have = acl_get_entry( acl, ACL_FIRST_ENTRY, &acl_entry );
  while (have == 1) {
    rb_yield( Data_Wrap_Struct( cEntry, 0, 0, acl_entry ) );
    have = acl_get_entry( acl, ACL_NEXT_ENTRY, &acl_entry );
  }
  if (have != 0) rb_sys_fail("acl_get_entry returned error");
  
  return self;
}

/* 
 * create new ACL object either by allocating new or duplicating given
 *    optional FIXNUM argument is passed to acl_init (number of entries to allocate)
 *
 * call-seq:
 *   ACL::new -> acl
 *   ACL::new( 10 ) -> acl
 *   ACL::new( acl ) -> acl
 *
 */
VALUE ruby_acl_new(int argc, VALUE *argv, VALUE class) {
  acl_t acl;
   
  switch (argc) {
    case 0:
      acl = acl_init(10);
      break;
    case 1: /* either copy constructor or given number of entries */
      if (FIXNUM_P(argv[0])) {
        if (NULL == (acl = acl_init( FIX2INT(argv[0]) ))) {
          rb_sys_fail("acl_init returned error");
        }
      } else {
        Data_Get_Struct_ex( argv[0], acl_t, acl );
        if (NULL == (acl = acl_dup( acl ))) {
          rb_sys_fail("acl_dup returned error");
        }
      }
      break;
    default:
      rb_raise(rb_eArgError, "wrong number of arguments (expect 0 or 1)");
  }
  return Data_Wrap_Struct( class, 0, ruby_acl_free, acl );
}

/*
 * create new ACL object from text representation of ACL
 *
 * call-seq:
 *   acl = ACL::from_text( string )
 *
 */
VALUE ruby_acl_from_text(VALUE class, VALUE text) {
  acl_t acl;
  if (NULL == (acl = acl_from_text( StringValuePtr(text) ))) {
    rb_sys_fail("acl_from_text returned error");
  }
  return Data_Wrap_Struct( class, 0, ruby_acl_free, acl );
}

/* 
 * create new ACL object from filename or file descriptor (access ACL)
 */
VALUE ruby_acl_from_file_fd(VALUE class, VALUE file) {
  acl_t acl;
  if (FIXNUM_P(file)) {
    acl = acl_get_fd( FIX2INT(file) );
    if ((acl_t)NULL==acl) rb_sys_fail("acl_get_fd failed");
  } else {
    acl = acl_get_file( StringValuePtr(file), ACL_TYPE_ACCESS );
    if ((acl_t)NULL==acl) rb_sys_fail("acl_get_file failed");
  }
  return Data_Wrap_Struct( class, 0, ruby_acl_free, acl );
}

/*
 * create new ACL object from directory name (default ACL)
 */
VALUE ruby_acl_from_file_default(VALUE class, VALUE dir) {
  acl_t acl;
  acl = acl_get_file( StringValuePtr(dir), ACL_TYPE_DEFAULT );
  if ((acl_t)NULL==acl) rb_sys_fail("acl_get_file failed");
  return Data_Wrap_Struct( class, 0, ruby_acl_free, acl );
}

/*
 * set ACL on file or fd
 */
VALUE ruby_acl_set_file( VALUE self, VALUE file ) {
  acl_t acl;
  
  Data_Get_Struct_ex( self, acl_t, acl );

  if (FIXNUM_P(file)) {
    if (0 != acl_set_fd(FIX2INT(file), acl) ) {
        rb_sys_fail("acl_set_fd failed");
    }
  } else {
    //if (0 != acl_set_file( StringValueCStr(file), ACL_TYPE_ACCESS, acl ) ) {
    if (0 != acl_set_file( StringValuePtr(file), ACL_TYPE_ACCESS, acl ) ) {
        rb_sys_fail("acl_set_file (ACCESS ACL) failed");
    }
  }
  return self;
}

/*
 * set default ACL on directory
 */
VALUE ruby_acl_set_default( VALUE self, VALUE dir ) {
  acl_t acl;
  
  Data_Get_Struct_ex( self, acl_t, acl );
  if (0 != acl_set_file( StringValuePtr(dir), ACL_TYPE_DEFAULT, acl ) ) {
        rb_sys_fail("acl_set_file (DEFAULT ACL) failed");
  }
  return self;
}

/*
 * check if ACL is valid
 * 
 * call-seq:
 *   acl.valid?
 *
 */
VALUE ruby_acl_valid( VALUE self ) {
  acl_t acl;

  Data_Get_Struct_ex( self, acl_t, acl );
  if (0 == acl_valid( acl )) {
    return Qtrue;
  } else {
    return Qfalse;
  }
}

/*
 * get ACL::Entry qualifier
 */
VALUE ruby_acl_entry_get_qualifier( VALUE self ) {
  acl_entry_t acl_entry;
  int *ret;
  VALUE val;
  
  Data_Get_Struct_ex( self, acl_entry_t, acl_entry );
  ret = acl_get_qualifier(acl_entry);
  if ( NULL == ret ) return Qnil;
  val = INT2FIX( *ret );
  acl_free( ret );
  return val;
}

/*
 * set ACL::Entry qualifier
 *
 * call-seq:
 *   entry.qualifier = uid
 *   entry.qualifier = gid
 *
 */
VALUE ruby_acl_entry_set_qualifier( VALUE self, VALUE val ) {
  int data;
  acl_entry_t acl_entry;
  Data_Get_Struct_ex( self, acl_entry_t, acl_entry );
  data = FIX2INT(val);
  if (0 != acl_set_qualifier(acl_entry, &data)) {
    rb_sys_fail("acl_set_qualifier failed");
  }
  return self;
}

/*
 * get ACL::Entry tag type
 */
VALUE ruby_acl_entry_get_tag_type( VALUE self ) {
  acl_entry_t acl_entry;
  acl_tag_t acl_tag;
  
  Data_Get_Struct_ex( self, acl_entry_t, acl_entry );
  if (0 != acl_get_tag_type(acl_entry, &acl_tag)) {
    rb_sys_fail("acl_set_qualifier failed");
  }
  return INT2FIX( acl_tag );
}

/*
 * set ACL::Entry tag type
 */
VALUE ruby_acl_entry_set_tag_type( VALUE self, VALUE val ) {
  acl_entry_t acl_entry;
  Data_Get_Struct_ex( self, acl_entry_t, acl_entry );
  if (0 != acl_set_tag_type(acl_entry, FIX2INT(val))) {
    rb_sys_fail("acl_set_tag_type failed");
  }
  return self;
}

/*
 * get ACL::Entry permisions
 */
VALUE ruby_acl_entry_get_permset( VALUE self ) {
  acl_entry_t acl_entry;
  acl_permset_t acl_permset;
  
  Data_Get_Struct_ex( self, acl_entry_t, acl_entry );
  if (0 != acl_get_permset(acl_entry, &acl_permset)) {
    rb_sys_fail("acl_get_permset failed");
  }
  return INT2FIX(*(int*)acl_permset);
}

/*
 * set ACL::Entry permisions
 */
VALUE ruby_acl_entry_set_permset( VALUE self, VALUE val ) {
  acl_entry_t acl_entry;
  acl_permset_t acl_permset;
  int perms;

  Data_Get_Struct_ex( self, acl_entry_t, acl_entry );

  if (0 != acl_get_permset(acl_entry, &acl_permset))
    rb_sys_fail("acl_get_permset failed");
  if (0 != acl_clear_perms(acl_permset))
    rb_sys_fail("acl_clear_perms failed");

  perms = FIX2INT(val);
  //#define set_perm(perm) if (0!=acl_add_perm(acl_permset,perm)) rb_sys_fail("acl_add_perm failed")
  //if (perms & 0x01) set_perm(ACL_EXECUTE);
  //if (perms & 0x02) set_perm(ACL_WRITE);
  //if (perms & 0x04) set_perm(ACL_READ);
  if (0!=acl_add_perm(acl_permset,perms))
    rb_sys_fail("acl_add_perm failed");

  if (0 != acl_set_permset(acl_entry, acl_permset )) {
    rb_sys_fail("acl_set_permset failed");
  }
  return self;
}

/*
 * copy ACL::Entry into another
 *   entries can be in different ACLs
 *
 * call-seq:
 *   dest_entry = source_entry
 *
 */
VALUE ruby_acl_copy_entry( VALUE self, VALUE source ) {
  acl_entry_t acl_dest, acl_source;

  Data_Get_Struct_ex( self,   acl_entry_t, acl_dest );
  Data_Get_Struct_ex( source, acl_entry_t, acl_source );

  if (0 != acl_copy_entry(acl_dest, acl_source)) {
    rb_sys_fail("acl_copy_entry failed");
  }
  return self;
}

/*
 *  Create new entry in acl
 *
 *  call-seq:
 *    entry = acl.create_entry
 */
VALUE ruby_acl_create_entry( VALUE self ) {
  acl_t acl;
  acl_entry_t acl_entry;

  Data_Get_Struct_ex( self, acl_t, acl );

  if (0 != acl_create_entry( &acl, &acl_entry ) ) {
    rb_sys_fail("acl_create_entry failed");
  }
  /* store the acl back into self */
  DATA_PTR(self) = (void*)acl;
  return Data_Wrap_Struct( cEntry, 0, 0, acl_entry );
}

/*
 *  Detele entry in ACL
 *
 *  call-seq:
 *    acl.delete_entry( entry )
 */
VALUE ruby_acl_delete_entry( VALUE self, VALUE entry ) {
  acl_t acl;
  acl_entry_t acl_entry;
  Data_Get_Struct_ex( self, acl_t, acl );
  Data_Get_Struct_ex( entry, acl_entry_t, acl_entry );

  if (0 != acl_delete_entry( acl, acl_entry ) ) {
    rb_sys_fail("acl_create_entry failed");
  }
  return self;
}

/*
 *  Document-class: ACL
 *  ACL -- interface to POSIX ACLs
 *
 *  ACL represents access controll list associated with a file or directory.
 *
 *  ACL instances are returned by methods ACL::from_text(text), ACL::from_file(file)
 *  or ACL::default(dir). 
 *  Changes to ACL instances are not reflected back. You have to call ACL#set_file(file) or
 *  ACL#set_default(dir) to applly changes back to the filesystem.
 */

/*
 *  Document-class: ACL::Entry
 *  ACL::Entry represents single entry of ACL.
 *
 *  Entries have the formL tag_type, permset, qualifier.
 */
void Init_acl(void) {
  cACL   = rb_define_class( "ACL", rb_cObject );
  cEntry = rb_define_class_under( cACL, "Entry", rb_cObject );

  /* Constructors */
  rb_define_singleton_method( cACL, "new",       ruby_acl_new,              -1 );
  rb_define_singleton_method( cACL, "from_text", ruby_acl_from_text,         1 );
  rb_define_singleton_method( cACL, "from_file", ruby_acl_from_file_fd,      1 );
  rb_define_singleton_method( cACL, "default",   ruby_acl_from_file_default, 1 );

  /* Setting ACL */
  rb_define_method( cACL, "set_file",     ruby_acl_set_file,     1 );
  rb_define_method( cACL, "set_default",  ruby_acl_set_default,  1 );
  
  /* Other methods */
  rb_define_method( cACL, "to_text",      ruby_acl_to_text,      0 );
  rb_define_method( cACL, "each",         ruby_acl_each,         0 );
  rb_define_method( cACL, "valid?",       ruby_acl_valid,        0 );

  rb_define_method( cACL, "create_entry", ruby_acl_create_entry, 0 );
  rb_define_method( cACL, "delete_entry", ruby_acl_delete_entry, 1 );

  rb_include_module( cACL, rb_mEnumerable );

  /* Manipulating with ACL entry */
  rb_define_method( cEntry, "qualifier",  ruby_acl_entry_get_qualifier, 0 );
  rb_define_method( cEntry, "qualifier=", ruby_acl_entry_set_qualifier, 1 );
  rb_define_method( cEntry, "tag_type",   ruby_acl_entry_get_tag_type,  0 );
  rb_define_method( cEntry, "tag_type=",  ruby_acl_entry_set_tag_type,  1 );
  rb_define_method( cEntry, "permset",    ruby_acl_entry_get_permset,   0 );
  rb_define_method( cEntry, "permset=",   ruby_acl_entry_set_permset,   1 );
  rb_define_method( cEntry, "=",          ruby_acl_copy_entry,          1 );
  /*
   * not implementing:
   * acl_add_perm(3), acl_clear_perms(3), acl_delete_perm(3), acl_get_perm(3),
   */

  /* Tag type constants */
  rb_define_const( cACL, "ACL_UNDEFINED_TAG", INT2FIX(ACL_UNDEFINED_TAG) );
  rb_define_const( cACL, "ACL_USER_OBJ"     , INT2FIX(ACL_USER_OBJ)      );
  rb_define_const( cACL, "ACL_USER"         , INT2FIX(ACL_USER)          );
  rb_define_const( cACL, "ACL_GROUP_OBJ"    , INT2FIX(ACL_GROUP_OBJ)     );
  rb_define_const( cACL, "ACL_GROUP"        , INT2FIX(ACL_GROUP)         );
  rb_define_const( cACL, "ACL_MASK"         , INT2FIX(ACL_MASK)          );
  rb_define_const( cACL, "ACL_OTHER"        , INT2FIX(ACL_OTHER)         );
  /* Permition constants */
  rb_define_const( cACL, "ACL_EXECUTE"      , INT2FIX(ACL_EXECUTE)       );
  rb_define_const( cACL, "ACL_WRITE"        , INT2FIX(ACL_WRITE)         );
  rb_define_const( cACL, "ACL_READ"         , INT2FIX(ACL_READ)          );
}

