provider tracecap_ruby_opentracing {
  probe ruby__span(uint64_t duration, const char *component, const char *description, struct ruby_span_extra *context);
};
struct ruby_span_extra {
  uint64_t ns_since_end;

  uint64_t rb_stack_len;
  const char *rb_stack;

  uint64_t metadata_len;
  const char *metadata;
};
