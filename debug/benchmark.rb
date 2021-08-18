require 'benchmark/ips'
require 'tracecap_opentelemetry'

def recurse(n, &block)
  if n > 0
    recurse(n-1, &block)
  else
    block.call()
  end
end

recurse(500) do # make the stack deep before benchmarking
  Benchmark.ips do |x|
    x.report("tracing") do
      TracecapOpenTelemetry::emit_span(0, 0, "hello", "world", "{}")
    end

    x.report("active check") do
      TracecapOpenTelemetry::active?
    end
  end
end
