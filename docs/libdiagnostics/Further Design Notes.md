# Further Design Notes

Here we consider the architecture in more detail.

Do note that halfway through our thought process we concluded that this diagnostics library is rather more *telemetry*-like than *logging*: while both are arguably interchangeable concepts, I merely note that my thinking has moved from calling this a *logging* library towards calling this a *telemetry* library approach. Don't ask me why, for these are fundamentally the same thing; it's probably due to the fact that I consider having the ability to have this data feed available as a Grafana source makes it *telemetry* in my mind, rather than *logging*.

I don't know yet how I can get Grafana to visualize images and/or arbitrary BLOBs as part of timeline events and click-accessible directly from the graphs produced by Grafana et al, but that's where this thing is heading: Grafana and friends as the preferred means for visualizing the data we are getting fed as *diagnostics data*, which is supposed to be a mix of text lines (like regular old-fashioned logging) and raw data elements, including images (and *image extracts*, i.e. small images which 'represent' some rectangular area of the original source image).



## log message attributes

When we think of a single log message as a (database) ROW, what are the COLUMNS we want to see? Which ones are relevant for any or all of these purposes:

- **filtering the log stream for reasons of**: (1) **performance**: fewer rows means less transfer and formatting costs; (2) **storage space**: fewer rows means reduced storage costs and possibly improved compressibility alongside.
- **reproducing the fact**: this requires us to unambiguously identify the application and its build / run configuration, such as config files, command line arguments and the git commit hash identifier for every git repo used to produce this particular executable.
- **filtering the log stream for reasons of**: (3) reducing surplus noise while drilling down into a particular issue, i.e. when you are debugging/diagnosing an issue, you often want to look into a particular section of the run-time/codebase at extremely elevated detail, while surrounding sections are of no or little interest and thus are (*temporarily*) considered *noise*. 
  These filters can, as a side effect, be used to keep *performance* high, as you may only require the details of a specific code section, while the other parts of the run-time merit a few "logging" data rows only... unless you wish to look into a different section for the same run, later on (as your diagnostic focus shifts): then the source must have produced all these rows as well! 
  **Consequently there's to be filtering at the *source* and at the (visualization) *destination*.**


For these purposes we expect these attributes to matter:

- loglevel (classic logging)
- call / execution hierarchy at run-time:
  1. category: a human name for a major section of the application
  2. chapter: a subsection of a category: a large section of the application
  3.  section
  4.  function -- shouldn't we support this level as a function call hierarchy?

  The nouns "category / chapter / section" are chosen as they make sense to me now as terms such as "task / module / component" already come with several overloaded meanings and use in other CS areas of interest, so I'd rather stick with less overloaded terms, borrowed from biology or book publishing.

  The question about *&function **hierarchy** tracking* leads to one having to consider call loops and *recursion*, i.e. how far do we want to take this thing? All the way to tracking the entire call tree? (That is assuming the absence of asynchronous operations, such as requiring the diagnostics tracer to be able to track message exchange across thread boundaries and/or coroutines and similar means of coding such things. In other words: **we are asking ourselves whether we track the calling sequences at run-time or are we intent on tracking particular data/messages through the run-time system**: the latter is not identical to the former as we consider multithreaded applications and/or coroutines usage in our codebase...!

  We consider these kind of call loops and/or recursive calls and wonder: do we want to be able to select the inner `C()` call level only? What's our run-time filter granularity?

  ```
    A() --> C()
    B() --> C()
    // do we filter/query the parental path that led to us being called?

	C() --> C()
	// simple recursion: do we want to select only the inner(most) call?
	// do we want to select only specific recursive call depths?

    A() --> C() --> B() --> A() --> A()
    // ditto as above, but now for more complicated recursions / call cycles.

    // select a specific order of execution, e.g.:
    A() --> C()               // C in A
    
    B(); C()
    X(); B(); C()             // C after X
    
    Y(); H(); C()
    Y(); C()                  // C immediately after Y

    // ... and what joy if, instead, we want to select: C /BEFORE/ A!
    ```


Note that last one, which requires the run-time to log every `C` at high detail (plus all `A` invocations), store/cache all of these and then, finally, only deliver the `C` rows which happen to occur before the first (or last?) `A`: here we observe the obvious need for two filtering stages:

1. **filtering as early as possible to keep the diagnostics output stream and encoding efforts as minimal as possible** -- theoretically, the above example filter queries can be optimized into this category, all except the last one as "*C before A*" requires look-ahead a.k.a. buffering: we *could* do that, but that's near-equivalent to tracking all `C` (and `A` invocations) in a database table and filtering that cache/table in "*post*"): if you want low memory and CPU overhead at run-time, the latter might be a simpler and cheaper approach.
2. PLUS additional filtering late in the diagnostics visualization process: that's where we have all the data stored already and our `C before A` filter query merely needs to travel the stored data at leisure -- that's us assuming that the viz process is allowed more leeway in spending time on filtering than our source application's run-time.



## Why would we bother with "early filtering" at all?

I expect some diagnostics tracts to be noticeably expensive in either CPU (message formatting) or storage (sampled attributes such as image snapshots and large BLOBs) or *both*; it should therefor be possible for the (expert?) user to command a reduced telemetry gathering effort to help speed up the actual *useful process* performed by the source application!

For instance, imagine tracing the detailed progress of a tesseract OCR operation executed on a scanned page image: this will end up producing many lines of text, each a separate (cropped) image snippet, which is split into word- and possibly character-sized sub-images, all of which will be logged at least once, but far more probably in *multiple* incantations as the OCR engines goes through its paces and attempt a few different things for each "*word*" as the page happens to be a rather complicated/noisy one. This results in a fair torrent of (compressed?!?!) image data for a single page, as part of the diagnostics/logging effort.
Now multiply this veritable diagnostics data *flood* by the number of pages in a single book which is being OCR-ed and you'll quickly realize we WILL need some knobs to twiddle to at least provide *some* modicum of control over (OCR) application performance and on-disk storage space requirements!

Hence the **late filtering is like regular database querying: providing us selection power** during diagnostics review/viz/use, while the **early filtering is a necessary means to tweak overall application performance to within acceptable bounds**, which reduces our selection power as some data will *not* be logged in the telemetry database when skipped thanks to the early filters *for performance reasons*.


### What would these filter queries look like in code?

Can we use SQLite query expressions for this? => Could this look like some SQL syntax?[^lc]


[^lc]: We intend to use SQL in various other parts of the larger (Qiqqa) application, so keeping the number of DSLs (Domain Specific Languages) low is a boon re end-user learning curves. Hence our preference for SQL syntax here.


Can we *compile* these query expressions for faster evaluation? Can we run these filter expressions through an *optimizer*, such that we end up with fast, compiled, filter queries for both early and late stage querying?
See the `C before A` example use case discussed above: an optimizer would instruct us, the application run-time, to log all `C` at the requested high detail, *plus* all `A` call events, *plus* produce a *second* filter query, this one tailored for the *late* stage, where the telemetry data table is filtered to dig up the first/last (?which one do we want, actually?) `A` call event, then seek back from there to dig up all `C` that went before it during the same *run*. Together these two filtering queries would then be able to deliver an optimized `C before A` experience at the diagnostics UX end point.




## Odd thought: could we treat the logging stream as an SQLite *virtual table*...

.... which can be queried => query run duration is the entire application run-time, or can SQLite "*stream*" these query results as it goes through the (potentially *infinite*) table?

    SELECT ... FROM ... TOP 5

=> early stop as we can stop producing logging once we have 5 rows of vetted output.

     SELECT ... FROM ...

=> run till end --> *paging* these results? --> "*stream*" data produced from SQLite queries possible?

Granted, having an SQL query like that for the *entire duration of the application* is rather counter-intuitive to database users/programmers, who would code this sort of thing as a *stored procedure* or *trigger* where the constructed SQL statement is applied once to every original source row: I don't know whether that's going to be acceptable as usually that sort of thing is slowing down `INSERT` database table/index performance to a crawl, unless one takes special care to keep these buggers to the leanest. meanest *minimum* cost!

    INSERT ... WHERE ...

might not be the fastest kid on the block, is what I'm saying...
... and is *why* I am wondering whether we can get away with some other means of filter expression application that comes close in expressive power, while not suffering from the potential *stored procedure* dreaded costs *per row* -- and we expect *many* and *many very expensive* diagnostics rows, which SHOULD be filtered at the early stage when we don't expect to need them for diagnostics/review/debugging the application processes!






## run-time performance: coding the diagnostics' calls

     logic-check() && logging-action();
 
where `logic-check()` *quickly* produces a boolean indicating whether the diagnostics filing, that comes next, is actually needed. C++ compiles this as a *lazy evaluation*, so we MAY encode all our heavy diagnostics lifting in the `logging-action()`, which might include lambda functions and other means to help make sure any additional activity, such as image sampling, encoding, BLOB encoding, (image) data compression, etc., is only actually executed when `logic-check()` says it's got to happen.

Incidentally, previous experience (with a hacked test rig) has taught us that image *encoding*, particularly in the (tight!) WEBP format, takes **significant CPU time**, so we are well advised to postpone that one until we really, *really* need it!

Also note that we wish to associate sub-images with their source/parent image, e.g. tesseract word/character image extracts from a scanned source page image. This implies that each image comes with additional metadata = attributes:

- size (dimensions: {w, h} pixels)
- offset in source (coordinate: {x, y} pixels)
- reference to source image





## Misc. notes (to be organized)


    log source --> filters (early) --> storage DB --> SQL queries (late filters: user queries, viz, diagnostics end user)


`log source` is a potentially infinite table: it's rather a *stream* of log data.

`early filters` is there for performance reasons and to keep storage DB for the telemetry to a manageable size.

Should we use a telemetry engine in the log piping instead of bothering with hacking with/inside SQLite?

Thinking about Grafana as our main viz end point, what is a smart thing to do here?

Questions about Grafana et al:

We have diagnostics data in the form of (1) logging lines, i.e. TEXT, (2) raw data and (3) image (snippets): how does Grafana cope with nos. 1 and 3 in their viz/dashboard? Is that good enough for our purposes? Do we need to spend effort on enhancing/tweaking Grafana to make this happen?

We already know we WILL need this extra Grafana plugin/tweak/hack effort if we want images to show up as part of any timeline graph/chart. Additional custom effort is required if we want to show any of those image snippets as part of the larger source image, i.e. show their proper place in time and (page) image location, as the OCR process goes through its paces.

How do we approach the issue of keeping data *post-partum*? Is Grafana responsible for deleting old data rows (I think not!) or do we ditch those old rows periodically when we run the process application again?
We WILL need to destroy old rows at some point as we want to keep the diagnostics DB small / manageable!

logging is telemetry; logging is a consequence of telemetry. logging is *one* implementation of telemetry.

The amount of telemetry output is determined by *rough* performance filters (compiled/fast).
Question: is this a separate expression engine, e.g. `exprtk`, or (ab-)using part of the SQL query parser / engine?
My personal preference is to use SQLite as we want to use that one as a source for and alternative for Grafana (not everybody might want to run Grafana, but use the SQLite CLI instead).

Watch out! Such filter queries are running for the duration of the process application and that's not what SQL folks are used to! Check *stored procedures* and *events* instead; but those are one query exec per row, which is bloody expensive!


How to *identify* a *build* and a *run*? --> id is a checksum, but of multiple git libs --> an id is a series of git commit hashes, one for each repo/lib used during the production of the build, all bundled into a single identifying *hash*; *reverse lookup* is mandatory for reproduction of the results as you must be able to produce and check the active git commit hash for each part involved in the build!

==> every run is a new *rowid*, which represents a (variable) series of library commit hashes and possibly other IDs such as hashes of language model databases, etc. used by the application  => id points at another table which stores these values, PLUS timestamp, PLUS command line used (as that impacts the run-time as well (*duh!*), PLUS config files (dumped) PLUS anything else that has any kind of influence on the application execution path/behaviour.
*This is your answer for how to reproduce the observed results!*

Log to SQLite DB; let Grafana query SQLite "*after the fact*" => SQLite is your telemetry database, with multiple tables for that purpose. (1 log record/row table; 1 component + build id table)



When considering this as a telemetry approach, using telemetry libraries:

- [Grafana and OpenTelemetry Integration](https://grafana.com/oss/opentelemetry/)
- [OpenTelemetry for C++](https://github.com/open-telemetry/opentelemetry-cpp)
- [Microsoft 1DS telemetry](https://github.com/microsoft/cpp_client_telemetry)
- https://github.com/open-telemetry/opentelemetry-cpp-contrib
- https://github.com/matthewmartin117/TelemetryVisualization + https://github.com/SFML/SFML
- https://github.com/dorsal-lab/opentelemetry-c
- https://github.com/zeek/zeek/wiki/Telemetry + https://github.com/zeek/zeek











