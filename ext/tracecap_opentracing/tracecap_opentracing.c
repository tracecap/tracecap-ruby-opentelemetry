#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include <ruby/ruby.h>
#include <ruby/debug.h>

#include "probes.h"

#define BUF_SIZE 2048

struct ruby_span_extra {
  uint64_t ns_since_end;

  uint64_t rb_stack_len;
  const char *rb_stack;

  uint64_t metadata_len;
  const char *metadata;
};

VALUE rb_tracecap_emit_span(VALUE self, VALUE duration, VALUE since_end, VALUE component, VALUE description, VALUE metadata)
{
  if (TRACECAP_RUBY_OPENTRACING_RUBY_SPAN_ENABLED()) {
    uint64_t duration_ns = NUM2ULL(duration);
    uint64_t since_end_ns = NUM2ULL(since_end);
    const char *component_c = StringValueCStr(component);
    const char *description_c = StringValueCStr(description);

    // compute the stack trace
    static VALUE stack_written_frames[BUF_SIZE];
    static int stack_written_lines[BUF_SIZE];
    static char *stack_written_offsets[BUF_SIZE];
    static int stack_last_depth = 0;
    static char stack[81920];
    struct ruby_span_extra span_extra;
    int stack_left = sizeof(stack);
    int num;
    VALUE frames_buffer[BUF_SIZE];
    int lines_buffer[BUF_SIZE];

    num = rb_profile_frames(0, sizeof(frames_buffer) / sizeof(VALUE), frames_buffer, lines_buffer);

    int same = 0;
    char *stack_curr = stack;
    for (int i = 0; i < num && i < stack_last_depth; i++) {
      if (frames_buffer[num - 1 - i] == stack_written_frames[i] && lines_buffer[num - 1 - i] == stack_written_lines[i])
        same++;
      else
        break;
    }

    if (same > 0) {
      stack_curr = stack_written_offsets[same - 1];
      stack_left = (int)(stack + sizeof(stack) - stack_curr);
    }

    int i;
    for (i = same; i < num; i++) {
      VALUE frame = frames_buffer[num - 1 - i];
      int line = lines_buffer[num - 1 - i];

      stack_written_frames[i] = frame;
      stack_written_lines[i] = line;

      VALUE name = rb_profile_frame_full_label(frame);
      VALUE file = rb_profile_frame_path(frame);

      if (stack_left > 0) {
        int n = snprintf(stack_curr, stack_left, "%s:%d:%s\n", StringValueCStr(file), line, StringValueCStr(name));
        stack_left -= n;
        stack_curr += n;
      } else {
        break;
      }

      stack_written_offsets[i] = stack_curr;
    }

    *stack_curr = '\0';
    stack_last_depth = i;

    /*
      probe ruby__span(uint64_t duration, const char *component, const char *description, struct ruby_span_extra *context);

      We could optionally adjust `since_end` here to compensate for any additional time spent in this function.
    */
    span_extra.ns_since_end = since_end_ns;
    span_extra.rb_stack_len = (int)(stack_curr - stack);
    span_extra.rb_stack = stack;
    span_extra.metadata_len = (int)RSTRING_LEN(metadata);
    span_extra.metadata = StringValuePtr(metadata);
    TRACECAP_RUBY_OPENTRACING_RUBY_SPAN(duration_ns, component_c, description_c, &span_extra);
  }
  return Qnil;
}

VALUE
rb_tracecap_is_active(VALUE self)
{
  return TRACECAP_RUBY_OPENTRACING_RUBY_SPAN_ENABLED() ? Qtrue : Qfalse;
}

void
Init_tracecap_opentracing(void) {
  VALUE tracecap_opentracing;
  tracecap_opentracing = rb_const_get(rb_cObject, rb_intern("TracecapOpenTracing"));

  rb_define_singleton_method(tracecap_opentracing, "emit_span", rb_tracecap_emit_span, 5);
  rb_define_singleton_method(tracecap_opentracing, "active?", rb_tracecap_is_active, 0);
}
