module TracecapOpenTracing
  class Tracer
    def initialize(proxy_tracer: nil)
      @proxy_tracer = proxy_tracer
    end

    def start_span(operation_name, *args, child_of: nil, start_time: Time.now, tags: nil, tracecap_only: false, **kwargs)
      proxy_span = if @proxy_tracer.nil? || tracecap_only
        nil
      else
        @proxy_tracer.start_span(operation_name, *args, child_of: child_of, start_time: start_time, tags: tags, **kwargs)
      end

      TracecapOpenTracing::Span.new(
        proxy_span: proxy_span,
        operation_name: operation_name,
        child_of: child_of,
        start_time: start_time,
        tags: tags,
      ).tap do |span|
        return yield span if block_given?
      end
    end

    def inject(*args, **kwargs)
      @proxy_tracer.inject(*args, **kwargs) if @proxy_tracer
    end

    def extract(*args, **kwargs)
      @proxy_tracer.extract(*args, **kwargs) if @proxy_tracer
    end

    def flush
      @proxy_tracer.flush if @proxy_tracer
    end
  end
end
