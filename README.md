# svn-copy
Tool for copying changes from one SVN location to another one.

Just a quick script for a one-time task.
Syntax: svn-copy source destination start-revision:finish-revision

"source" and "destination" should point to working copies of the
corresponding locations in SVN repositories.

Destination working copy should have no content and no pending
modifications.

Only files are supported, if subdirectory will be encountered,
probably some error will happen.