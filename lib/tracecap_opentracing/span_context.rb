module TracecapOpenTracing
  class SpanContext
    attr_reader :id, :trace_id, :parent_id
  
    def initialize(id:, trace_id:, parent_id:)
      @id = id
      @trace_id = trace_id
      @parent_id = parent_id
    end
  end
end
