require 'test/unit'
require 'stringio'

$LOAD_PATH.unshift('ext/genx4r')

require 'genx4r'

# $Id: basics.rb 268 2004-07-10 00:33:11Z rooneg $

class BasicsTest < Test::Unit::TestCase
  def init_writer
    return GenX::Writer.new, StringIO.new
  end

  def test_element
    w, io = init_writer

    w.document(io) do
      w.element("http://bar", "foo") do
      end
    end

    assert io.string == "<g1:foo xmlns:g1=\"http://bar\"></g1:foo>"

    w, io = init_writer

    w.document(io) do
      w.element("foo") do
      end
    end

    assert io.string == "<foo></foo>"

    w, io = init_writer

    w.begin_document(io)
    w.begin_element("foo")
    w.text("bar")
    w.end_element
    w.end_document

    assert io.string == "<foo>bar</foo>"
  end

  def test_nested_elements
    w, io = init_writer

    w.document(io) do
      w.element("foo") do
        w.element("bar") do
        end
      end
    end

    assert io.string == "<foo><bar></bar></foo>"
  end

  def test_text
    w, io = init_writer

    w.document(io) do
      w.element("foo") do
        w.text("bar")
      end
    end

    assert io.string == "<foo>bar</foo>"
  end

  def test_comment
    w, io = init_writer

    w.document(io) do
      w.element("foo") do
        w.comment("blah")
      end
    end

    assert io.string == "<foo><!--blah--></foo>"
  end

  def test_pi
    w, io = init_writer

    w.document(io) do
      w.element("base") do
        w.pi("foo", "bar")
      end
    end

    assert io.string == "<base><?foo bar?></base>"
  end
end
