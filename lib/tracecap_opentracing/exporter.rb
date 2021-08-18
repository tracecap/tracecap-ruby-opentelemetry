# frozen_string_literal: true

require 'opentelemetry/sdk'

module TracecapOpenTelemetry
  class Exporter < OpenTelemetry::SDK::Trace::Export::InMemorySpanExporter # pretend to be this for now for 'simple_exporter?' check
    def initialize
      @stopped = false
    end

    def export(spans, timeout: nil)
      return FAILURE if @stopped

      return SUCCESS unless TracecapOpenTelemetry::active?

      Array(spans).each do |span|
        tags = span.attributes
        component = tags["db.system"] || tags["component"] || "Trace"

        operation = span.name

        context = tags.dup
        context["span_id"] = span.hex_span_id
        context["span_parent_id"] = span.hex_parent_span_id if span.hex_parent_span_id != "0000000000000000"
        json_ctx = JSON.dump(context)

        start_time_nsec = span.start_timestamp.to_i
        end_time_nsec = span.end_timestamp.to_i
        now_nsec = (Time.now.to_r * 1_000_000_000).to_i
        duration = end_time_nsec - start_time_nsec
        end_delta = now_nsec - end_time_nsec

        TracecapOpenTelemetry::emit_span(duration, end_delta, component.to_s, operation.to_s, json_ctx)
      end

      SUCCESS
    end

    def force_flush(timeout: nil)
      SUCCESS
    end

    def shutdown(timeout: nil)
      @stopped = true
      SUCCESS
    end
  end
end
