## Code Comments Platform

This platform allow the upload of supported code files (.cpp, .h, .py, .ts) and output all comments within each file, their line numbers and the ability to edit the comments and save the file with the altered comment structure preserving original file format. The platform features a GUI aspect for easy file addition and code modification that does not involve having to deal with CLI or changing code files.

### Issues Solved

This platform ensures code commenting throughout files is up-to-date, sufficient and accurate by showing all the comments so you can skim through them easily or look at them in detail when necessary. For inline comments the comment usually refers to the code on that line so the whole line is shown. The platform works on the assumption that the user knows what the code is doing therefore the code around the comment is not needed (inline is an exception). It should allow to skim through the comments to see any TODO: or any blatantly obsolete/wrong comments, whilst allowing for slow systematic read of them all and the ability to edit them as needed. This will ensure larger projects are properly documented and removes the need of scrolling through 1000s of lines of code whilst checking comment structure.

## Tech Stack

This platform is built using C++ with Qt6 library.