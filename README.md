# libdiagnostics

A C/C++ diagnostics logging library with support for writing HTML and store images and misc binary data to disk alongside. or when a mere logfile doesn't suffice.


## Why & when use `libdiagnostics`?

- when you want to log not just some simple log lines, but expect to output a related set of images and/or other binary data for future inspection.

  > Example: my *augmented* `tesseract` OCR engine where I want & need to see the internals at work more closely, *after the fact* (as we'll be batch-processing image pages): I want to see the images and the extracts used by `tesseract` as it moves along. A log file doesn't cut it, by half!
  >
  > The classic "solution" was to run a `Scrollview` hacky somewhat-interactive JAVA app in parallel with the tesseract run you wished to inspect more closely. Definitely not suited for batch processing or other *post-fact*/*post-mortem* diagnostics scenarios!
  > 
  > `libdiagnostics` instead writes an intellible HTML file which links to every image produced internally during the run.
  > And you have a HTML file per session, so you can inspect those at your leasure some time afterwards when you have got time to do some digging!

- when you like to use a log library that's both fast and flexible and able to produce both HTML and regular pure-text log files (when you find HTML is putting a crimp on your `grep` or `vim` Foo ;-) )


## API

- init
- log
  - text line
  - info chunk (text)
  - image (leptonica)
  - image (OpenCV)
  - binary data (...)
- finish
- cycle  (others would call this `rotate` but it's more than that; it finalizes the current diagnostics output file and starts a new one; only when set up that way does it *rotate* and thus overwrite older session output files)
- config (change the configuration on the fly; the initial configuration is done in the init call)


