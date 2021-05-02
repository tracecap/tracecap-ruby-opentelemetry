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

static struct {
  VALUE stack_written_frames[BUF_SIZE];
  int stack_written_lines[BUF_SIZE];
  char *stack_written_offsets[BUF_SIZE];
  int stack_last_depth;
  char stack[81920];

  VALUE frames_buffer[BUF_SIZE];
  int lines_buffer[BUF_SIZE];
} _tracecap_data;

static inline uint64_t tracecap_update_stack()
{
  int num;
  int stack_left = sizeof(_tracecap_data.stack);

  // read in the latest set of frames/lines
  num = rb_profile_frames(0, sizeof(_tracecap_data.frames_buffer) / sizeof(VALUE), _tracecap_data.frames_buffer, _tracecap_data.lines_buffer);

  // work out how many are the same as previously calculated
  int same = 0;
  char *stack_curr = _tracecap_data.stack;
  for (int i = 0; i < num && i < _tracecap_data.stack_last_depth; i++) {
    if (_tracecap_data.frames_buffer[num - 1 - i] == _tracecap_data.stack_written_frames[i] && _tracecap_data.lines_buffer[num - 1 - i] == _tracecap_data.stack_written_lines[i])
      same++;
    else
      break;
  }

  if (same > 0) {
    stack_curr = _tracecap_data.stack_written_offsets[same - 1];
    stack_left = (int)(_tracecap_data.stack + sizeof(_tracecap_data.stack) - stack_curr);
  }

  int i;
  for (i = same; i < num; i++) {
    VALUE frame = _tracecap_data.frames_buffer[num - 1 - i];
    int line = _tracecap_data.lines_buffer[num - 1 - i];

    _tracecap_data.stack_written_frames[i] = frame;
    _tracecap_data.stack_written_lines[i] = line;

    VALUE name = rb_profile_frame_full_label(frame);
    VALUE file = rb_profile_frame_path(frame);
    char *file_str = StringValueCStr(file);
    char maybe_package[128] = {0};
    char *gems_slash = strstr(file_str, "/gems/");
    char *next = gems_slash;
    
    // find the last occurance of /gems/
    while (next != NULL && *next != 0) {
      next = strstr(next + 1, "/gems/");
      if (next != NULL) {
        gems_slash = next;
      }
    }

    if (gems_slash != NULL) {
      char *gem_name_start = gems_slash + strlen("/gems/");
      char *inside_gem = strchr(gem_name_start, '/');
      if (inside_gem != NULL) {
        file_str = inside_gem + 1; // skip over '/'
        snprintf(maybe_package, sizeof(maybe_package), "%.*s:", (int)(inside_gem - gem_name_start), gem_name_start);
      }
    }

    if (stack_left > 0) {
      int n = snprintf(stack_curr, stack_left, "%s%s:%d:%s\n", maybe_package, file_str, line, StringValueCStr(name));
      stack_left -= n;
      stack_curr += n;
    } else {
      break;
    }

    _tracecap_data.stack_written_offsets[i] = stack_curr;
  }

  *stack_curr = '\0';
  _tracecap_data.stack_last_depth = i;

  return (uint64_t)(stack_curr - _tracecap_data.stack);
}

VALUE rb_tracecap_emit_span(VALUE self, VALUE duration, VALUE since_end, VALUE component, VALUE description, VALUE metadata)
{
  if (TRACECAP_RUBY_OPENTRACING_RUBY_SPAN_ENABLED()) {
    uint64_t duration_ns = NUM2ULL(duration);
    uint64_t since_end_ns = NUM2ULL(since_end);
    const char *component_c = StringValueCStr(component);
    const char *description_c = StringValueCStr(description);

    struct ruby_span_extra span_extra;

    int stack_len = tracecap_update_stack();

    /*
      probe ruby__span(uint64_t duration, const char *component, const char *description, struct ruby_span_extra *context);
      We could optionally adjust `since_end` here to compensate for any additional time spent in this function.
    */
    span_extra.ns_since_end = since_end_ns;
    span_extra.rb_stack_len = stack_len;
    span_extra.rb_stack = _tracecap_data.stack;
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

  _tracecap_data.stack_last_depth = 0;

  tracecap_opentracing = rb_const_get(rb_cObject, rb_intern("TracecapOpenTracing"));
  rb_define_singleton_method(tracecap_opentracing, "emit_span", rb_tracecap_emit_span, 5);
  rb_define_singleton_method(tracecap_opentracing, "active?", rb_tracecap_is_active, 0);
}
