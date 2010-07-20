require 'test/unit'
require 'stringio'

$LOAD_PATH.unshift('ext/genx4r')

require 'genx4r'

# $Id: declare.rb 491 2004-11-25 03:26:54Z rooneg $

class DeclareTest < Test::Unit::TestCase
  def init_writer
    return GenX::Writer.new, StringIO.new
  end

  def test_element
    w, io = init_writer

    e = w.declare_element("bar")

    w.document(io) do
      w.element(e) do
      end
    end

    assert io.string == "<bar></bar>"
  end

  def test_attribute
    w, io = init_writer

    a = w.declare_attribute("blah")

    w.document(io) do
      w.element("foo") do
        w.attribute(a, "baz")
      end
    end

    assert io.string == "<foo blah=\"baz\"></foo>"
  end

  def test_namespace
    w, io = init_writer

    n = w.declare_namespace("http://foo.com/bar", "bar")

    e = w.declare_element(n, "blah")
    a = w.declare_attribute(n, "zot")

    w.document(io) do
      w.element(e) do
        w.attribute(a, "zang")
      end
    end

    assert io.string == "<bar:blah xmlns:bar=\"http://foo.com/bar\"" + 
                        " bar:zot=\"zang\"></bar:blah>"

    assert_raise(ArgumentError) do
      w.declare_namespace
    end
  end
end
