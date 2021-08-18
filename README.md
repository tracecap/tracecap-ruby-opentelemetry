# tracecap-ruby-opentelemetry

Exports OpenTelemetry-compatible spans via USDT for tracecap ingestion.

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'tracecap_opentelemetry'
```

And then execute:

    $ bundle install

Or install it yourself as:

    $ gem install tracecap_opentelemetry

## Usage

To emit traces, use `TracecapOpenTelemetry::Exporter` as an OpenTelemetry exporter. `TracecapOpenTelemetry::active?` can also be used to see if a tracer is listening.

The spans can be traced directly with `dtrace`:
```
$ sudo dtrace -n 'ruby-span { trace(arg0); trace(copyinstr(arg1)); trace(copyinstr(arg2)); }'
```

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/tracecap/tracecap-ruby-opentelemetry.

## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).
