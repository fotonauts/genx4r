require 'genx4r'

# $Id: builder.rb 449 2004-10-02 20:15:49Z rooneg $

module GenX
  class Blank
    instance_methods.each do |m|
      undef_method m unless m =~ /^(__|instance_eval)/
    end
  end

  class Builder < Blank
    def initialize(writer)
      @writer = writer

      @elements   = {}
      @attributes = {}
      @namespaces = {}

      @indent = 2
      @level  = 0
    end

    def method_missing(sym, *args, &block)
      text  = nil
      attrs = nil

      args.each do |arg|
        case arg
        when Hash
          attrs ||= {}
          attrs.merge!(arg)
        else
          text ||= ''
          text << arg.to_s
        end
      end

      _cache_element(sym)
      _cache_attrs(attrs) unless attrs.nil?

      if block
        raise ArgumentError, 'can\'t have both text and block' unless text.nil?
        _indent
        @writer.element(@elements[sym]) do
          _write_attrs(attrs) unless attrs.nil?
          _write_contents(block)
        end
        _newline
      else
        _indent
        @writer.element(@elements[sym]) do
          _write_attrs(attrs) unless attrs.nil?
          @writer.text(text)  unless text.nil?
        end
        _newline
      end
    end

    def _write_contents(block)
      begin
        @level += 1
        _newline
        block.call(self)
      ensure
        @level -= 1
        _indent
      end
    end

    def _write_attrs(attrs)
      attrs.each_key do |attr|
        @writer.attribute(@attributes[attr], attrs[attr])
      end
    end

    def _cache_attrs(attrs)
      attrs.each_key do |attr|
        unless @attributes.has_key?(attr)
          @attributes[attr] = @writer.declare_attribute(attr.to_s)
        end
      end
    end

    def _cache_element(sym)
      unless @elements.has_key?(sym.to_s)
        @elements[sym] = @writer.declare_element(sym.to_s)
      end
    end

    def _newline
      unless @indent == 0 || @level == 0
        @writer.text("\n")
      end
    end

    def _indent
      unless @indent == 0 || @level == 0
        @writer.text(" " * (@indent * @level))
      end
    end

    def _begin_doc(io)
      @writer.begin_document(io)
    end

    def _end_doc
      @writer.end_document
    end
  end

  class Writer
    def builder(io, &block)
      b = Builder.new(self)
      begin
        b._begin_doc(io)
        block.call(b)
      ensure
        b._end_doc
      end
    end
  end
end
