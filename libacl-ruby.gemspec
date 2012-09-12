require File.expand_path('../lib/acl/version', __FILE__)

#http://guides.rubygems.org/c-extensions/

Gem::Specification.new do |s|
  s.name        = "acl"
  s.version     = ACL::VERSION
  s.summary     = "Posix ACL bindings"
  s.description = "Bindings for POSIX ACL"
  s.homepage    = "https://github.com/martinpovolny/libacl-ruby"
  s.authors     = ["Martin Povolny"]
  s.email       = ["martin.povolny@gmail.com"]
  
  #s.add_runtime_dependency 'builder'

  s.files       = `git ls-files '*.c' '*.rb' '*.h'`.split("\n")
  s.extensions  = ['ext/acl/extconf.rb']

  #s.files         = `git ls-files`.split("\n")
  #s.executables   = `git ls-files -- bin/*`.split("\n").map{|f| File.basename(f)}

  #s.add_dependency('fileutils')
  s.has_rdoc = true
  s.extra_rdoc_files = ['ext/acl/acl.c']

  
  s.require_paths = ["lib"]
end
