require 'test/unit'

$LOAD_PATH.unshift('ext/genx4r')
$LOAD_PATH.unshift('lib')

require 'genx4r/builder'

# $Id: builder.rb 493 2004-11-25 15:17:36Z rooneg $

class BuilderTest < Test::Unit::TestCase
  def test_basics
    w = GenX::Writer.new()

    s = ''

    w.builder(s) do |b|
      b.foo
    end

    assert s == '<foo></foo>'

    s = ''

    w.builder(s) do |b|
      b.foo('bar')
    end

    assert s == '<foo>bar</foo>'

    s = ''

    w.builder(s) do |b|
      b.foo do
        b.bar('baz')
        b.baz('zot', 'foo' => 'bar')
        b.zim
      end
    end

    # the \n is just so this works with the here doc...
    assert (s + "\n")  == <<EOF
<foo>
  <bar>baz</bar>
  <baz foo="bar">zot</baz>
  <zim></zim>
</foo>
EOF
  end
end
