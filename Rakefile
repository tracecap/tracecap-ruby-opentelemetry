require "rake/extensiontask"
require "bundler/gem_tasks"
task :default => :spec

Rake::ExtensionTask.new "tracecap_opentelemetry" do |ext|
  ext.lib_dir = "lib/tracecap_opentelemetry"
end
