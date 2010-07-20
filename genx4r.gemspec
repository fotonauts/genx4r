require 'rubygems'

spec = Gem::Specification.new do |s|
  s.name    = 'genx4r-fotopedia'
  s.version = '0.6'

  s.summary = <<-EOF
    GenX4r is a Ruby wrapper around the GenX library, which allows you to
    programatically generate correct, cannonical XML output.
  EOF

  s.description = <<-EOF
    GenX4r is a Ruby wrapper around the GenX library, which allows you to
    programatically generate correct, cannonical XML output.
  EOF

  s.files = Dir.glob("**/*").delete_if { |item| item.include?(".svn") }

  s.extensions << "ext/genx4r/extconf.rb"

  s.require_path = 'lib'

  s.autorequire = 'genx4r'

  s.has_rdoc = false

  s.test_files = Dir.glob('test/*.rb')

  s.authors            = ["Garrett Rooney", "Pierre Baillet"]
  s.email             = "rooneg@electricjellyfish.net"
  s.homepage          = "http://genx4r.rubyforge.org"
  s.rubyforge_project = "genx4r"
end
