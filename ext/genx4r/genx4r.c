/* Copyright (c) 2004 Garrett Rooney
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

/* $Id: genx4r.c 494 2004-11-25 15:21:41Z rooneg $ */

#include "ruby.h"

#include "genx.h"

static VALUE rb_mGenX;
static VALUE rb_cGenXWriter;

static VALUE rb_cGenXNamespace;
static VALUE rb_cGenXAttribute;
static VALUE rb_cGenXElement;

static VALUE rb_cGenXException;

static void
writer_mark (genxWriter w)
{}

static void
writer_free (genxWriter w)
{
  genxDispose (w);
}

#define GENX4R_ERR(expr, w)                                         \
  do {                                                              \
    genxStatus genx4r__status = (expr);                             \
    if (genx4r__status)                                             \
      rb_raise (rb_cGenXException, "%s", genxLastErrorMessage (w)); \
  } while (0)

static VALUE
call_write (VALUE ary)
{
  rb_funcall (rb_ary_entry (ary, 0),
              rb_intern ("<<"),
              1,
              rb_ary_entry (ary, 1));

  return Qtrue;
}

static VALUE
call_flush (VALUE file)
{
  rb_funcall (file, rb_intern ("flush"), 0);

  return Qtrue;
}

static VALUE
handle_exception (VALUE unused)
{
  return Qfalse;
}

static genxStatus
writer_send (void *baton, constUtf8 s)
{
  VALUE file = (VALUE) baton;
  VALUE ary;

  if (! rb_respond_to (file, rb_intern ("<<")))
    rb_raise (rb_eRuntimeError, "target must respond to '<<'");

  ary = rb_ary_new2 (2);

  rb_ary_store (ary, 0, file);
  rb_ary_store (ary, 1, rb_str_new2 (s));

  if (rb_rescue (call_write, ary, handle_exception, Qnil) == Qfalse)
    return GENX_IO_ERROR;
  else
    return GENX_SUCCESS;
}

static genxStatus
writer_send_bounded (void *baton, constUtf8 start, constUtf8 end)
{
  VALUE file = (VALUE) baton;
  VALUE ary;

  if (! rb_respond_to (file, rb_intern ("<<")))
    rb_raise (rb_eRuntimeError, "target must respond to '<<'");

  ary = rb_ary_new2 (2);

  rb_ary_store (ary, 0, file);
  rb_ary_store (ary, 1, rb_str_new (start, end - start));

  if (rb_rescue (call_write, ary, handle_exception, Qnil) == Qfalse)
    return GENX_IO_ERROR;
  else
    return GENX_SUCCESS;
}

static genxStatus
writer_flush (void *baton)
{
  VALUE file = (VALUE) baton;

  /* if we can't flush, just let it go... */
  if (! rb_respond_to (file, rb_intern ("flush")))
    return GENX_SUCCESS;

  if (rb_rescue (call_flush, file, handle_exception, Qnil) == Qfalse)
    return GENX_IO_ERROR;
  else
    return GENX_SUCCESS;
}

static genxSender writer_sender = { writer_send,
                                    writer_send_bounded,
                                    writer_flush };

static VALUE
writer_begin_document (VALUE self, VALUE file)
{
  genxWriter w;

  Data_Get_Struct (self, struct genxWriter_rec, w);

  if (! rb_respond_to (file, rb_intern ("<<")))
    rb_raise (rb_eRuntimeError, "target must respond to '<<'");

  genxSetUserData(w, (void *) file);

  GENX4R_ERR (genxStartDocSender (w, &writer_sender), w);

  return Qnil;
}

static VALUE
writer_end_document (VALUE self)
{
  genxWriter w;

  Data_Get_Struct (self, struct genxWriter_rec, w);

  GENX4R_ERR (genxEndDocument (w), w);

  return Qnil;
}

static VALUE
writer_document (VALUE self, VALUE file)
{
  writer_begin_document (self, file);

  if (rb_block_given_p ())
    {
      rb_yield (Qnil);

      writer_end_document (self);

      return Qnil;
    }
  else
    rb_raise (rb_eRuntimeError, "must be called with a block");
}

static VALUE
writer_comment (VALUE self, VALUE arg)
{
  genxWriter w;

  Check_Type (arg, T_STRING);

  Data_Get_Struct (self, struct genxWriter_rec, w);

  GENX4R_ERR (genxComment (w, (constUtf8) RSTRING (arg)->ptr), w);

  return Qnil;
}

static VALUE
writer_attribute (int argc, VALUE *argv, VALUE self)
{
  genxAttribute attr = 0;
  genxWriter w;
  VALUE xmlns, name, value;
  int nslen = 0;

  switch (argc)
    {
    case 2:
      xmlns = 0;
      if (CLASS_OF (argv[0]) == rb_cGenXAttribute)
        {
          Data_Get_Struct (argv[0], struct genxAttribute_rec, attr);
          name = 0;
        }
      else
        name = argv[0];

      value = argv[1];
      break;

    case 3:
      xmlns = argv[0];
      name = argv[1];
      value = argv[2];
      break;

    default:
      rb_raise (rb_eRuntimeError, "invalid arguments");
    }

  if (xmlns)
    {
      Check_Type (xmlns, T_STRING);
      nslen = RSTRING (xmlns)->len;
    }

  if (name)
    Check_Type (name, T_STRING);

  Check_Type (value, T_STRING);

  Data_Get_Struct (self, struct genxWriter_rec, w);

  if (attr)
    GENX4R_ERR (genxAddAttribute (attr, (constUtf8) RSTRING (value)->ptr), w);
  else
    GENX4R_ERR (genxAddAttributeLiteral
                 (w,
                  nslen ?  (constUtf8) RSTRING (xmlns)->ptr : NULL,
                  (constUtf8) RSTRING (name)->ptr,
                  (constUtf8) RSTRING (value)->ptr), w);

  return Qnil;
}

static VALUE
writer_begin_element (int argc, VALUE *argv, VALUE self)
{
  genxWriter w;
  genxElement elem = 0;
  VALUE xmlns, name;
  int nslen = 0;

  switch (argc)
    {
    case 1:
      xmlns = 0;
      if (CLASS_OF (argv[0]) == rb_cGenXElement)
        {
          Data_Get_Struct (argv[0], struct genxElement_rec, elem);
          name = 0;
        }
      else
        name = argv[0];
      break;

    case 2:
      xmlns = argv[0];
      name = argv[1];
      break;

    default:
      rb_raise (rb_eRuntimeError, "invalid arguments");
    }

  if (xmlns)
    {
      Check_Type (xmlns, T_STRING);
      nslen = RSTRING (xmlns)->len;
    }

  if (name)
    Check_Type (name, T_STRING);

  Data_Get_Struct (self, struct genxWriter_rec, w);

  if (elem)
    GENX4R_ERR (genxStartElement (elem), w);
  else
    GENX4R_ERR (genxStartElementLiteral
                 (w,
                  nslen ? (constUtf8) RSTRING (xmlns)->ptr : NULL,
                  (constUtf8) RSTRING (name)->ptr), w);

  return Qnil;
}

static VALUE
writer_end_element (VALUE self)
{
  genxWriter w;

  Data_Get_Struct (self, struct genxWriter_rec, w);

  GENX4R_ERR (genxEndElement (w), w);

  return Qnil;
}

static VALUE
writer_element (int argc, VALUE *argv, VALUE self)
{
  writer_begin_element (argc, argv, self);

  if (rb_block_given_p ())
    {
      rb_yield (Qnil);

      writer_end_element (self);

      return Qnil;
    }
  else
    rb_raise (rb_eRuntimeError, "must be called with a block");
}

static VALUE
writer_pi (VALUE self, VALUE target, VALUE text)
{
  genxWriter w;

  Check_Type (target, T_STRING);
  Check_Type (text, T_STRING);

  Data_Get_Struct (self, struct genxWriter_rec, w);

  GENX4R_ERR (genxPI (w,
                      (constUtf8) RSTRING (target)->ptr,
                      (constUtf8) RSTRING (text)->ptr), w);

  return Qnil;
}

static VALUE
writer_text (VALUE self, VALUE text)
{
  genxWriter w;

  Check_Type (text, T_STRING);

  Data_Get_Struct (self, struct genxWriter_rec, w);

  GENX4R_ERR (genxAddText (w, (constUtf8) RSTRING (text)->ptr), w);

  return Qnil;
}

static VALUE
writer_character (VALUE self, VALUE ch)
{
  genxWriter w;

  int c = NUM2INT (ch);

  Data_Get_Struct (self, struct genxWriter_rec, w);

  GENX4R_ERR (genxAddCharacter (w, c), w);

  return Qnil;
}

#if RUBY_VERSION_MAJOR >= 1 && RUBY_VERSION_MINOR >= 8
static VALUE
writer_allocate (VALUE klass)
{
  genxWriter writer = genxNew (NULL, NULL, NULL);

  return Data_Wrap_Struct (klass, writer_mark, writer_free, writer);
}
#else
static VALUE
writer_new (VALUE klass)
{
  genxWriter writer = genxNew (NULL, NULL, NULL);

  return Data_Wrap_Struct (klass, writer_mark, writer_free, writer);
}
#endif

static VALUE
writer_declare_namespace (int argc, VALUE *argv, VALUE self)
{
  VALUE uri, prefix = 0;
  genxStatus st = 0;
  genxNamespace ns;
  genxWriter w;

  rb_scan_args (argc, argv, "11", &uri, &prefix);

  Check_Type (uri, T_STRING);
  if (prefix)
    Check_Type (prefix, T_STRING);

  Data_Get_Struct (self, struct genxWriter_rec, w);

  ns = genxDeclareNamespace (w,
                             (constUtf8) RSTRING (uri)->ptr,
                             prefix ? (constUtf8) RSTRING (prefix)->ptr : NULL,
                             &st);

  if (st)
    rb_raise (rb_cGenXException, "%s", genxGetErrorMessage (w, st));

  return Data_Wrap_Struct (rb_cGenXNamespace, NULL, NULL, ns);
}

static VALUE
writer_declare_element (int argc, VALUE *argv, VALUE self)
{
  genxNamespace ns = 0;
  genxStatus st = 0;
  genxElement elem;
  genxWriter w;
  VALUE name;

  switch (argc)
    {
    case 1:
      name = argv[0];
      break;

    case 2:
      if (CLASS_OF (argv[0]) == rb_cGenXNamespace) {
        Data_Get_Struct (argv[0], struct genxNamespace_rec, ns);
      } else {
        rb_raise (rb_eRuntimeError, "invalid arguments");
      }
      name = argv[1];
      break;

    default:
      rb_raise (rb_eRuntimeError, "invalid arguments");
    }

  Check_Type (name, T_STRING);

  Data_Get_Struct (self, struct genxWriter_rec, w);

  elem = genxDeclareElement (w, ns, RSTRING (name)->ptr, &st);
  if (st)
    rb_raise (rb_cGenXException, "%s", genxGetErrorMessage (w, st));

  return Data_Wrap_Struct (rb_cGenXElement, NULL, NULL, elem);
}

static VALUE
writer_declare_attribute (int argc, VALUE *argv, VALUE self)
{
  genxNamespace ns = 0;
  genxAttribute attr;
  genxStatus st = 0;
  genxWriter w;
  VALUE name;

  switch (argc)
    {
    case 1:
      name = argv[0];
      break;

    case 2:
      if (CLASS_OF (argv[0]) == rb_cGenXNamespace) {
        Data_Get_Struct (argv[0], struct genxNamespace_rec, ns);
      } else {
        rb_raise (rb_eRuntimeError, "invalid arguments");
      }
      name = argv[1];
      break;

    default:
      rb_raise (rb_eRuntimeError, "invalid arguments");
    }

  Check_Type (name, T_STRING);

  Data_Get_Struct (self, struct genxWriter_rec, w);

  attr = genxDeclareAttribute (w, ns, RSTRING (name)->ptr, &st);
  if (st)
    rb_raise (rb_cGenXException, "%s", genxGetErrorMessage (w, st));

  return Data_Wrap_Struct (rb_cGenXAttribute, NULL, NULL, attr);
}

void
Init_genx4r ()
{
  rb_mGenX = rb_define_module ("GenX");

  rb_cGenXWriter = rb_define_class_under (rb_mGenX, "Writer", rb_cObject);

#if RUBY_VERSION_MAJOR >= 1 && RUBY_VERSION_MINOR >= 8
  rb_define_alloc_func (rb_cGenXWriter, writer_allocate);
#else
  rb_define_singleton_method(rb_cGenXWriter, "new", writer_new, 0);
#endif

  rb_define_method (rb_cGenXWriter, "document", writer_document, 1);

  rb_define_method (rb_cGenXWriter,
                    "begin_document",
                    writer_begin_document,
                    1);

  rb_define_method (rb_cGenXWriter,
                    "end_document",
                    writer_end_document,
                    0);

  rb_define_method (rb_cGenXWriter, "comment", writer_comment, 1);
  rb_define_method (rb_cGenXWriter, "pi", writer_pi, 2);

  rb_define_method (rb_cGenXWriter, "element", writer_element, -1);

  rb_define_method (rb_cGenXWriter, "begin_element", writer_begin_element, -1);
  rb_define_method (rb_cGenXWriter, "end_element", writer_end_element, 0);

  rb_define_method (rb_cGenXWriter, "attribute", writer_attribute, -1);
  rb_define_method (rb_cGenXWriter, "text", writer_text, 1);
  rb_define_method (rb_cGenXWriter, "character", writer_character, 1);

  rb_define_method (rb_cGenXWriter,
                    "declare_namespace",
                    writer_declare_namespace,
                    -1);

  rb_define_method (rb_cGenXWriter,
                    "declare_element",
                    writer_declare_element,
                    -1);

  rb_define_method (rb_cGenXWriter,
                    "declare_attribute",
                    writer_declare_attribute,
                    -1);

  rb_cGenXNamespace = rb_define_class_under (rb_mGenX,
                                             "Namespace",
                                             rb_cObject);

  rb_cGenXElement = rb_define_class_under (rb_mGenX, "Element", rb_cObject);

  rb_cGenXAttribute = rb_define_class_under (rb_mGenX,
                                             "Attribute",
                                             rb_cObject);

  rb_cGenXException = rb_define_class_under (rb_mGenX,
                                             "Exception",
                                             rb_eStandardError);
}
