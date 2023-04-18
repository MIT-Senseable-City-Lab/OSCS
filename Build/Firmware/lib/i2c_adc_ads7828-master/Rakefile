# encoding: utf-8
#
# Copyright:: 2009-2016 Doc Walker
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

require 'git'
require 'github_changelog_generator/task'
require 'rake'
require 'rubygems'
require 'rake/version_task'         # gem install version
require 'version'

# requires additional packages on MacOS (including Homebrew):
# $ /usr/bin/ruby -e "$(curl -fsSL \
#   https://raw.githubusercontent.com/Homebrew/install/master/install)"
# $ brew install doxygen        # generates documentation from source code
# $ brew cask install mactex    # MacTeX

Rake::VersionTask.new do |task|
  # prevent auto-commit on version bump
  task.with_git = false
end

# adjust as appropriate
DOXYFILE        = 'Doxyfile'
GITHUB_USERNAME = '4-20ma'
GITHUB_REPO     = 'i2c_adc_ads7828'
HEADER_FILE     = "#{GITHUB_REPO}.h"
CHANGELOG       = 'CHANGELOG'
CHANGELOG_FILE  = "#{CHANGELOG}.md"
PROPERTIES_FILE = 'library.properties'
VERSION_FILE    = Version.version_file('').basename.to_s


task :default => :info

desc 'Display instructions for public release'
task :info do
  puts <<-EOF.gsub(/^\s{2}/, '')
  
  Instructions for public release
  
  - Update version, as appropriate:
  
    $ rake version:bump           # or
    $ rake version:bump:minor     # or
    $ rake version:bump:major     # or
    edit 'VERSION' file directly
    
  - Prepare release date, '#{CHANGELOG_FILE}' file, documentation:
  
    $ rake prepare
    
  - Review changes to '#{CHANGELOG_FILE}' file
    This file is assembled using git commit messages; review for completeness.
  
  - Review html documentation files
    These files are assembled using source code Doxygen tags; review for
    for completeness.
  
  - Add & commit source files, tag, push to origin/master;
    add & commit documentation files, push to origin/gh-pages:
  
    $ rake release
    
  EOF
end # task :info


desc "Prepare #{CHANGELOG_FILE} for release"
task :prepare => 'prepare:default'

namespace :prepare do
  task :default => %w(release_date library_properties changelog documentation)

  desc 'Prepare documentation'
  task :documentation => :first_time do
    version = Version.current.to_s

    # update parameters in Doxyfile
    cwd = File.expand_path(__dir__)
    file = File.join(cwd, 'doc', DOXYFILE)

    contents = IO.read(file)
    contents.sub!(/(^PROJECT_NUMBER\s*=)(.*)$/) do |match|
      "#{$1} v#{version}"
    end # contents.sub!(...)
    IO.write(file, contents)

    # chdir to doc/ and call doxygen to update documentation
    Dir.chdir(to = File.join(cwd, 'doc'))
    system('doxygen', DOXYFILE)

    # chdir to doc/latex and call doxygen to update documentation
    Dir.chdir(from = File.join(cwd, 'doc', 'latex'))
    system('make')

    # move/rename file to 'extras/GITHUB_REPO reference-x.y.pdf'
    to = File.join(cwd, 'extras')
    FileUtils.mv(File.join(from, 'refman.pdf'),
      File.join(to, "#{GITHUB_REPO} reference-#{version}.pdf"))
  end # task :documentation

  # desc 'Prepare doc/html directory (first-time only)'
  task :first_time do
    cwd = File.expand_path(File.join(__dir__, 'doc', 'html'))
    FileUtils.mkdir_p(cwd)
    Dir.chdir(cwd)

    # skip if this operation has already been completed
    next if 'refs/heads/gh-pages' == `git config branch.gh-pages.merge`.chomp

    # configure git remote/branch options
    origin = "https://github.com/#{GITHUB_USERNAME}/#{GITHUB_REPO}"
    `git init`
    `git remote add origin #{origin}`
    `git checkout --orphan gh-pages`
    `git config --replace-all branch.gh-pages.remote origin`
    `git config --replace-all branch.gh-pages.merge refs/heads/gh-pages`
    `touch index.html`
    `git add .`
    `git commit -a -m 'Initial commit'`
  end

  desc 'Prepare release history'
  GitHubChangelogGenerator::RakeTask.new(:changelog) do |config|
    config.add_issues_wo_labels = false
    config.add_pr_wo_labels = false
    config.enhancement_labels = [
      'Type: Enhancement',
      'Type: Feature Request'
    ]
    config.enhancement_prefix = 'IMPROVEMENTS'
    config.bug_labels = ['Type: Bug']
    config.bug_prefix = 'BUG FIXES'
    config.future_release = "v#{Version.current.to_s}"
    config.header = "# #{GITHUB_REPO} #{CHANGELOG}"
    config.include_labels = [CHANGELOG]
    config.merge_prefix = 'OTHER' # e.g. 'Type: Maintenance'
    config.project = GITHUB_REPO
    config.user = GITHUB_USERNAME
  end # GitHubChangelogGenerator::RakeTask.new

  desc 'Update version in library properties file'
  task :library_properties do
    version = Version.current.to_s

    cwd = File.expand_path(__dir__)
    file = File.join(cwd, PROPERTIES_FILE)

    contents = IO.read(file)
    contents.sub!(/(version=\s*)(.*)$/) do |match|
      "#{$1}#{version}"
    end # contents.sub!(...)
    IO.write(file, contents)
  end # task :library_properties

  desc 'Update release date in header file'
  task :release_date do
    cwd = File.expand_path(__dir__)
    file = File.join(cwd, 'src', HEADER_FILE)

    contents = IO.read(file)
    contents.sub!(/(\\date\s*)(.*)$/) do |match|
      "#{$1}#{Time.now.strftime('%-d %b %Y')}"
    end # contents.sub!(...)
    IO.write(file, contents)
  end # task :release_date

end # namespace :prepare


desc 'Release source & documentation'
task :release => 'release:default'

namespace :release do
  task :default => %w(source documentation)

  desc 'Commit documentation changes related to version bump'
  task :documentation do
    version = Version.current.to_s
    cwd = File.expand_path(File.join(__dir__, 'doc', 'html'))
    g = Git.open(cwd)

    # `git add .`
    g.add

    # remove each deleted item
    g.status.deleted.each do |item|
      g.remove(item[0])
    end # g.status.deleted.each

    # commit changes if items added, changed, or deleted
    if g.status.added.size > 0 || g.status.changed.size > 0 ||
      g.status.deleted.size > 0 then
      message = "Update documentation for v#{version}"
      puts g.commit(message)
    else
      puts "No changes to commit v#{version}"
    end # if g.status.added.size > 0 || g.status.changed.size > 0...

    g.push('origin', 'gh-pages')
  end # task :documentation

  desc 'Commit source changes related to version bump'
  task :source do
    version = Version.current.to_s
    `git add \
      doc/#{DOXYFILE} \
      "extras/#{GITHUB_REPO} reference-#{version}.pdf" \
      src/#{HEADER_FILE} \
      #{CHANGELOG_FILE} \
      #{PROPERTIES_FILE} \
      #{VERSION_FILE} \
    `
    `git commit -m 'Version bump to v#{version}'`
    `git tag -a -f -m 'Version v#{version}' v#{version}`
    `git push origin master`
    `git push --tags`
  end # task :source

end # namespace :release
