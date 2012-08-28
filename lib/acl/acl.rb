require 'acl.so'

class File
  def acl
    ACL.from_file(fileno)
  end

  def default_acl
    ACL.default(fileno)
  end

  def acl=(acl)
    acl.set_file(fileno)
  end

  def default_acl=(acl)
    acl.set_default(fileno)
  end
end
