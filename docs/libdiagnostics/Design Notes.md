# Design Notes

> **Notice**: These are design notes for the library under development and SHOULD NOT be perceived as documentation / feature set / API of the final product.


## Accepted *inputs*

- structured data (suitable for [JSONL](https://jsonlines.org/), hence simple structures, not cyclic state machines and such)
- images / binary data BLOBs

Inputs (calls to log via the library) should provide these attributes/data elements:

- source
	- IP address? -- we're not running multi-homed so that bit is quite unimportant now, but I know `syslog` et al requires it...
	- Application *name* -- task ids are cute for sysadmins but only useful for message bundling-by-machine later on
	- section -- a stack ("breadcrumb path"), more like it. `section/subsection/sub2/...` 
- severity -- use the syslog levels, more or less. Though I have never seen the use of 'fatal' vs. 'error': the last error is generally fatal when things go pear-shaped. glog (google-log) is a lib which has 'fatal' level as one of its options, but I think that's nuts as then the 'side effect' of the log library is calling `abort()` under the hood and no way to do something after the "*mayday! mayday! we're going down!*" message, while many 'fatals', while indeed fatal for the application, can sometimes be alleviated by attempting some last-minute action, e.g. closing files, etc., to lighten the disaster.
- Message
	- message text -- as usual...
	- data -- the 'structured stuff' you may want to include in the diagnostic: we're about *helpful* here, vs. *security* ("information is given on a need-to-know basis and you don't need to know"), the latter of which is often encountered in production environments to discourage hackers.
	- image(s)?
	- binary data BLOBs?

## Expected *outputs*

- human readable text files -- a.k.a. classic "logfiles". Rotated, (compressed?), accessible with minimal tooling (text editor), ...
- human readable HTML files -- similar, but now we can go through them using regular web browsers and we can include links (`<img src="...">`), etc. to the accompanying image files for improved human understanding of what's going on. The bit that started all this, really, as I need/want a log "format" which deals with a mix of images and data/text, while monitoring/diagnosing `tesseract` et al.
- [JSONL](https://jsonlines.org/) -- for [grafana](https://github.com/grafana/grafana), et al: watching batch runs via [graylog](https://github.com/Graylog2/graylog2-server), [netdata](https://github.com/netdata/netdata), [loki](https://github.com/grafana/loki), [grafana](https://github.com/grafana/grafana), ...: charts vs. text overload to appease the human brain when bombarded with bulk diag info.
- GELF -- ditto, but this is the [graylog](https://github.com/Graylog2/graylog2-server) native format. Same bunch of targets. From what I read about it, some noises suggest this is JSONL with some steroids jacked up the ass. Not a major priority right now as I don't expect to use [graylog](https://github.com/Graylog2/graylog2-server), rather more [netdata](https://github.com/netdata/netdata) + [loki](https://github.com/grafana/loki), both feeding [grafana](https://github.com/grafana/grafana).
- syslog / syslog-ng -- notice the new RFC ([RFC5424](https://www.rfc-editor.org/info/rfc5424)) which has a little blurb added about structured data includes. Anyway, syslog ([RFC3164](https://www.rfc-editor.org/info/rfc3164)) is a nice generic format that's supported across the UNIX realm, so a good start for a feed into [loki](https://github.com/grafana/loki)/[netdata](https://github.com/netdata/netdata)/others, if I am any judge.
   
  There is a mention of the 1024 byte message limit though, which I'm not happy about.
* [MS Windows Debug Output Channel](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/sending-output-to-the-debugger) -- great stuff when you got it and use Visual Studio, like I do.
* MS Windows Event Log -- hola! Remember this one requires compiled resources and registry work to make the event log entries look good?! Been there, done that, but then and now, I've got the feeling nobody in their right mind is [using EventLog](https://stackoverflow.com/questions/8559222/write-an-event-to-the-event-viewer), so:
  
  This should be dealt with via an external "*intermediary*" application, which can take care of that registry + resource crap for us. Yeah, code duplication and all that jazz, so that's gonna be one very generic template resource entry set right there. Anyhow, I don't want to deal with this crap in the main application(s) that are using `libdiagnostics`. ÜBER-LOW PRIORITY because, like I said: nobody in their right mind...
* SQL / [SQLite](https://github.com/sqlite/sqlite)? -- cute, but that would mean we'ld be *sharing* a database across an application boundary. *Should* be fine but, given past experience, I'ld rather not get there and taunt the Lovecraftian Murphy Gods.


Looking at the above, the adjustable/pluggable log engine part splits into two categories:
- formatting ([JSONL](https://jsonlines.org/), text, [syslog message format](https://www.rfc-editor.org/info/rfc5424), etc.)
- transmit medium (file, socket (TCP/UDP) to [syslog daemon](https://github.com/syslog-ng/syslog-ng), [netdata](https://github.com/netdata/netdata), [loki](https://github.com/grafana/loki), [influxdb](https://github.com/influxdata/influxdb), ...)


Should we have an application/log-type identifier/prefix per medium? Expected: easier filtering at the receiving end as different medium *probably* means different needs --> different end-point / user filters for the same grand-total log stream that's pumped through `libdiagnostics`.


## How to cope with logged images and binary BLOBs?

per output channel (specific mix of formatting + transmit medium):

- configurable wrapper/formatting, e.g.:
	- HTML file: `href`to local file (or rather: `<img src="local/path...">`)
	- [JSONL](https://jsonlines.org/) or HTML or ... via socket transmit: include custom marker or URL format template so remote gatherer and human viewer can access the stored image files + binary files, which have to be reached through other means, e.g. additional static file serving webserver?


## Speed?

Provide a buffer for direct write/append; no string allocations on heap if we can help it; no intermediate serialization: structured data is encoded/processed only *once* per format.


## Backwards compatible API behaviour?

Provide a `printf`+ `tprintf`interface on top.

Might want to set some app-id / section / etc. defaults for when a library or part of application uses these 'generic (text) logging' calls. For speed reasons, it's not advised to inject special control codes in the messages themselves or some such to drive section/severity processing.

Hm. `tesseract` et al already do that in the current codebase, where we string-compare detect and dispatch/reroute under the hood. Guess we'll have to, anyway. However... either way is arguably code change at the user application level, so why not code-change to the new, more organized, log API then? Current opinion (which may change): change it to this way for potentially faster = less costly logging overall.


## Setup / config?

- should be adjustable on the fly, i.e. at run-time:
	- severity filters / routing
	- section filters / routing
	
   *Why* do we want this? Only needed rarely; generally you set up the logging channels and be done with it. However, when debugging particular sections of a larger app, you may want to *bump* the logging in the section/sections of interest.
   Hm. Wouldn't it be "better" then to be able to quick-config that *bump* at app start? Not if a section is visited many times and we only want raised log output for context-specific runs through those sections: the conditions may be complicated, so better to have some run-time "bump it up / lower it" API calls to help manage the amount of log output... next to the setup/config at app-start way.
   
- application can define *shortcuts* a.k.a. *aliases* for specific format + output combo's to quickly address them, e.g. turn on/off via config command.


## Working with `sections`

push + pop + pop_until_mark -- so we can play loose with the stack:

```
mark = push()
...
pop()
```

vs.

```
mark = push()
...
pop(mark)   // pop all subsections until mark
```

The former (first) would have its `pop()` result in unexpected behaviour if the section 'stack' is not pair-matched *everywhere* --> unexpected behaviour when going through a rough patch with exception handling and abortive stack unwinding, using C `setjmp`/`longjmp`, for example.

However, it's not nice having to store `mark` across complex runtime logic/code, hence this alternative:

```
push(id)  // human coder defines id
...
mark = find(id)  // locate(id): search active section stack
pop(mark)
```

which would then look something like this in code:

```
mark = push(id)
...
// code loses access to `mark`
...
mark = locate(id)
pop(mark)
```

while cleaner/leaner chunks of application code would have this reduced to:

```
mark = push(id)
...
pop(mark)
```

which could do with a *nil* or *auto*-id:

```
mark = push(-)   // no id collision as this is anonymous auto-id
...
pop(mark)
```



Risk: What if we push multiple of the same `id`? Either through collision (human-assigned `id` can occur in two or more places in large codebases), or, more probable, because code is recursive and section is recursively entered and exited?

Answer: use a push-counter + pop-decrement and pop-final, i.e. hard-reset counter to zero.
Might be handy to push *nil* scopes (sections), i.e. subsections which don't add section depth in the log reporting: these can be used for exception handling and consequent brutal stack unwinding: exception handling code can hard-reset the *nil* section's push counter without jeopardizing the outer section's counter state.
Something like this:

```
mark = push(id)   // actual section mark; push-counted
nilmark = push_nil(-) // nil subsection doesn't get listed in log output
try {
  ...
  throw!
}
catch {
  ...
}
finally {
  pop_nil(nilmark)  // unwinds section stack all the way to this level
}
...
pop(mark)   // pop section mark once we like to do so; section stack is OK
```

The above idea can be reduced to `pop-up-to-this-marker` i.e. `pop-all-our-children[but-not-us-ourselves]`:

```
mark = push(id)   // actual section mark; push-counted
try {
  ...
  throw!
}
catch {
  ...
}
finally {
  pop_children(mark)  // unwinds section stack all the way, UP TO this level
}
...
pop(mark)   // pop section mark once we like to do so; section stack is OK
```

and then, of course, there's the obvious C++ maneuver: have the *destructor* do the `pop`-ping... or the safe equivalent using a [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr):

```
// push:
std::unique_ptr<DiagnosticExecStackMarker> mark = std::make_unique<DiagnosticExecStackMarker>(id);   // actual section mark
try {
  ...
  throw!
}
catch {
  ...
}
finally {
  pop_children(mark)  // unwinds section stack all the way to this level
}
...
// pop section mark once we like to do so; section stack is OK
mark = null;
// OR:
// `mark` goes out of scope, i.e. std::unique_ptr auto-invokes the destructor.
```



## Extra Features

### Unique identifier for every log message

- create a quick code scan+rewrite tool which seeks out all these error messages and check+replace the identifiers so as to ensure that existing identifiers are kept as-is when unique and non-unique ones are "uniquified". `MSG0000` (or any chosen equivalent identifier text template/format) will be assigned a new, unique number when the `0000` number is zero(0): that way one can quickly write new log messages, assign then code 0 and then run the code inspect+rewrite tool to assign a sane identifier value/code.

Inspired by https://climbtheladder.com/10-c-logging-best-practices-2/  item #7:

> #### 7. Make sure that each message has a unique identifier
>
> When debugging an issue, it’s often necessary to search through log files for specific messages. If each message has a unique identifier, it makes it much easier to find the exact message you’re looking for. This is especially important when dealing with large log files that contain thousands of lines of text.
> 
> Unique identifiers also make it easier to track down issues in production environments. By having a unique identifier associated with each message, it’s possible to quickly identify which messages are related to a particular problem and trace them back to their source.



### Benchmarks / performance

Other logging libraries have published various benchmarks, which indicate some of the faster fellas out there -- though one has to reckon with benchmarks often being different from "reality" ;-) 

Meanwhile, https://github.com/chronoxor/CppLogging and https://github.com/SergiusTheBest/plog look promising as a starter / base block...

- https://github.com/chronoxor/CppBenchmark


logging libraries:

- https://github.com/google/glog
- https://github.com/SergiusTheBest/plog
- https://github.com/mjbots/log4cpp / https://sourceforge.net/p/log4cpp/codegit/ci/master/tree/
- https://github.com/log4cplus/log4cplus
- https://github.com/chronoxor/CppLogging
- https://codereview.stackexchange.com/questions/221922/c-logging-library : tip to use `boost::stacktrace::stacktrace();` when [stacktrace](https://www.boost.org/doc/libs/1_70_0/doc/html/stacktrace.html) is available on the platform and you want debug stack traces to go with some of your log lines. 
- 
- quill: fast, supports binary output or may use a custom sink that does this. async logging for low latency. Seems more suitable than spdlog, while using quite a few of the same (preferred) mechanics as spdlog: fmt, multiple sinks, multiple loggers, ...
