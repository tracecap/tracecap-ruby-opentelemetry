# tracecap-ruby-opentracing

Exports OpenTracing-compatible spans via USDT for tracecap ingestion.

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'tracecap_opentracing'
```

And then execute:

    $ bundle install

Or install it yourself as:

    $ gem install tracecap_opentracing

## Usage

To emit traces, use `TracecapOpenTracing::Tracer` as an OpenTracing::Tracer compatible tracer. Optionally, provide an upstream distributed tracing instance to proxy spans and other events through unless they have `tracecap_only: true` specified:
```ruby
tracer = TracecapOpenTracing::Tracer.new(proxy_tracer: distributed_tracer) # optionally proxy to a distributed tracer

span = tracer.start_span("example operation", tracecap_only: true) # only emit this to tracecap, not to the proxied tracer
# do something here
span.set_tag('component', 'example')
span.when_enabled do |span|
  span.set_tag('expensive_tag', calculate_expensive_tag())
end
span.finish
```

Emitting spans with `tracecap_only: true` will only be emitted via USDT when a tracing application like tracecap is actually listening. `TracecapOpenTracing::active?` can also be used to see if a tracer is listening.

The spans can be traced directly with `dtrace`:
```
$ sudo dtrace -n 'ruby-span { trace(arg0); trace(copyinstr(arg1)); trace(copyinstr(arg2)); }'
```

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/tracecap/tracecap-ruby-profiler.

## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).
