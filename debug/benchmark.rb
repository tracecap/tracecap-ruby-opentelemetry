require 'benchmark/ips'
require 'tracecap_opentracing'

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
      TracecapOpenTracing::emit_span(0, 0, "hello", "world", "blah")
    end

    x.report("active check") do
      TracecapOpenTracing::active?
    end
  end
end
