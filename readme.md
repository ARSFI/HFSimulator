<h2> A practical Open Source Ionospheric Channel Simulator for Ham use </h2>

Rick Muething KN6KB, Tom Lafleur KA6IQA, Tom Whiteside N5TW

This Article introduces a practical high-performance stand-alone and OS independent Ionospheric Channel
Simulator, full open source documentation is available here.

What is An Ionospheric Simulator?

Every ham learns that much of our communication relies heavily on radio wave propagation through the Ionosphere
layer. This layer reflects radio waves and can create single or multiple hop paths from station to station. While most
radio waves involve this type of propagation it is not trivial to model the constantly changing Ionosphere to test and
optimize the performance of our radios and protocols. On-air tests are notoriously unpredictable and difficult or
impossible to duplicate at another time.

In 1970 a landmark paper [1] by Watterson, Juroshek, and Bensema was presented in the IEEE Transaction on
Communication Technology. They described a mathematical process (model) that could fairly accurately model and
simulate radio propagation over narrow band HF. This and follow-on papers by others and the CCIR/ITU [2] defined a
set of standardized “representative test channels” that would allow computer software or hardware simulators to
statistically model HF radio propagation by manipulating the modulating audio. This has been used over the years to
develop, test, and improve many of the various digital and digital voice protocol we use today.

