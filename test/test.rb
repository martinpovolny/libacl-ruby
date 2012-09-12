require "acl"
require "test/unit"
require "fileutils"

class ACLTest < Test::Unit::TestCase
  TEST_PATH='/tmp/rtest'
  
  def test_constructors
    assert_instance_of( ACL, ACL.from_file(TEST_PATH) )
    assert_instance_of( ACL, ACL.default(TEST_PATH) )

    assert_instance_of( ACL, ACL.new )          # new acl
    assert_instance_of( ACL, a = ACL.new(10) )  # new acl with given numof entries
    assert_instance_of( ACL, b = ACL.new(a) )   # duplicate given acl

    assert_equal( a.to_text, b.to_text )
  end

  def test_to_from_text
    a = ACL.default(TEST_PATH)
    assert_instance_of( String, a.to_text )     # test if to_text gives String

    b = ACL.from_text( a.to_text )              # create new acl from string
    assert_instance_of( ACL, b )                # must by class ACL

    assert_equal( a.to_text, b.to_text )        # text reprezentation of both acl's must be the same
  end

  def test_entry
    entry = nil
    a = ACL.from_file(TEST_PATH).each do |e|
      entry = e
    end
    assert_instance_of( ACL::Entry, entry )     # take individual ACL entry
    assert_respond_to( entry, :qualifier )      # test existence of it's methods
    assert_respond_to( entry, :qualifier= )
    assert_respond_to( entry, :tag_type )
    assert_respond_to( entry, :tag_type= )
    assert_respond_to( entry, :permset )
    assert_respond_to( entry, :permset= )

    entry           = a.create_entry            # test creation of new ACL entry
    entry.tag_type  = ACL::ACL_USER
    entry.qualifier = 1
    entry.permset   = 7

    assert_equal( entry.tag_type,  ACL::ACL_USER )  # test the getter/setter methods
    assert_equal( entry.qualifier, 1 )
    assert_equal( entry.permset,   7 )
    
    # musime nastavit i masku!
    entry2          = a.create_entry
    entry2.tag_type  = ACL::ACL_MASK
    entry2.permset   = 7

    assert_equal( true, a.valid? )              # test call of the validity check function
  end

  def test_set
    acl = ACL.from_file(TEST_PATH)
    fname = File.join(TEST_PATH,'test')
    FileUtils.mkpath(fname)

    entry = acl.create_entry
    entry.tag_type  = ACL::ACL_USER
    entry.qualifier = 1
    entry.permset   = 7
    
    # musime nastavit i masku!
    entry2          = acl.create_entry
    entry2.tag_type  = ACL::ACL_MASK
    entry2.permset   = 7

    acl.set_file(fname)                     # test setting ACL on a file
    # TODO: test default acl

    acl2 = ACL.from_file(fname)             # test, that the ACL was correctly set 
    assert_equal( acl.to_text, acl2.to_text )
  end

  # ruby -racl -e 'a = ACL.from_file(TEST_PATH); puts a.to_text; a.each { |e| p e; p e.qualifier; p e.tag_type; p e.permset }'

end

FileUtils.mkpath(ACLTest::TEST_PATH)

