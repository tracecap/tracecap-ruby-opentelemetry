require_relative 'lib/tracecap_opentelemetry/version'

Gem::Specification.new do |spec|
  spec.name          = "tracecap_opentelemetry"
  spec.version       = TracecapOpenTelemetry::VERSION
  spec.authors       = ["Theo Julienne"]
  spec.email         = ["theo.julienne@gmail.com"]

  spec.summary       = %q{Exports OpenTelemetry spans via USDT for tracecap ingestion}
  spec.homepage      = "https://github.com/tracecap/tracecap-ruby-opentelemetry"
  spec.license       = "MIT"
  spec.required_ruby_version = Gem::Requirement.new(">= 2.3.0")

  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["source_code_uri"] = "https://github.com/tracecap/tracecap-ruby-opentelemetry"

  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
  spec.files         = Dir.chdir(File.expand_path('..', __FILE__)) do
    `git ls-files -z`.split("\x0").reject { |f| f.match(%r{^(test|spec|features)/}) }
  end
  spec.bindir        = "exe"
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ["lib"]

  spec.extensions << 'ext/tracecap_opentelemetry/extconf.rb'

  spec.add_development_dependency 'rake-compiler'
  spec.add_development_dependency 'benchmark-ips'
  
end
