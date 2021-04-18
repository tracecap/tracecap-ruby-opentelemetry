require 'securerandom'

module TracecapOpenTracing
  class Span
    attr_reader :start_time, :finish_time, :tags, :data, :context
    attr_accessor :operation_name

    def initialize(
      proxy_span: nil,
      operation_name:,
      child_of: nil,
      start_time: Time.now,
      tags: nil
    )
      @proxy_span = proxy_span
      @operation_name = operation_name
      @start_time = start_time
      @tags = {}

      id = SecureRandom.uuid

      trace_id = if child_of.nil?
        id
      else
        child_of.trace_id
      end

      @context = TracecapOpenTracing::SpanContext.new(
        id: id,
        trace_id: trace_id,
        parent_id: child_of ? child_of.id : nil,
      )

      unless tags.nil?
        tags.each do |k, v|
          set_tag(k, v)
        end
      end
    end

    def set_tag(key, value)
      @tags[key] = value

      @proxy_span.set_tag(key, value) if @proxy_span
    end

    def set_baggage_item(key, value)
      @proxy_span.set_baggage_item(key, value) if @proxy_span
    end

    def get_baggage_item(key)
      @proxy_span.get_baggage_item(key) if @proxy_span
    end

    def log(*args, **kwargs)
      @proxy_span.log(*args, **kwargs) if @proxy_span
    end

    def log_kv(*args, **kwargs)
      @proxy_span.log_kv(*args, **kwargs) if @proxy_span
    end

    def finish(end_time: Time.now)
      @end_time = end_time

      emit_span

      if @proxy_span
        @proxy_span.finish(end_time: end_time)
      else
        nil
      end
    end

    def when_enabled
      if TracecapOpenTracing::active?
        yield self
      elsif @proxy_span
        @proxy_span.when_enabled do
          yield self
        end
      end
    end

    private

    def emit_span
      return unless TracecapOpenTracing::active?

      component = if @tags['db.system']
        @tags['db.system']
      elsif @tags['component']
        @tags['component']
      else
        'Trace'
      end

      operation = @operation_name

      context = @tags.dup
      context['span_id'] = @context.id
      context['span_parent_id'] = @context.parent_id
      json_ctx = JSON.dump(context)
      
      start_time_nsec = (@start_time.to_r * 1_000_000_000).to_i
      end_time_nsec = (@end_time.to_r * 1_000_000_000).to_i
      now_nsec = (Time.now.to_r * 1_000_000_000).to_i
      duration = end_time_nsec - start_time_nsec
      end_delta = now_nsec - end_time_nsec

      TracecapOpenTracing::emit_span(duration, end_delta, component.to_s, operation.to_s, json_ctx)
    end
  end
end
