require 'mkmf'

have_library('acl','acl_get_file')
create_makefile("acl")

rdoc = find_executable0( 'rdoc' ) || find_executable0( 'rdoc1.8' )

File.open('Makefile', "a") do |f|
  f << <<EOS
doc: acl.c acl.rb
	#{rdoc} $^

dist: acl.c extconf.rb README copyright t/test.rb
	mkdir -p acl-ruby
	cp --parents $^ acl-ruby/
	tar cvjf acl.tar.bz2 acl-ruby/
	rm -rf acl-ruby

EOS
end
