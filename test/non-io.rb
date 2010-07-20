require 'test/unit'
require 'stringio'

$LOAD_PATH.unshift('ext/genx4r')

require 'genx4r'

# $Id: basics.rb 202 2004-06-13 21:41:59Z rooneg $

# some class that doesn't respond to <<
class SomethingElse
end

# tests that we can use anything that replies to <<, not just an IO object
class NonIoTest < Test::Unit::TestCase
  def test_non_io
    w = GenX::Writer.new
    s = String.new

    w.document(s) do
      w.element("http://bar", "foo") do
      end
    end

    assert s == "<g1:foo xmlns:g1=\"http://bar\"></g1:foo>"

    s = SomethingElse.new

    begin
      w.document(s) do
        w.element("foo") do
        end
      end
    rescue RuntimeError => re
      assert re.to_s == "target must respond to '<<'"
      return
    end

    fail "should have raised exception..."
  end
end
