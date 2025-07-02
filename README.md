## Code Comments Platform

This platform will allow the upload of supported code files (.cpp, .h, .py, .ts ...) and output all comments within each file, their line numbers and the ability to edit the comments and save the file with the altered comment structure. The platform should feature a GUI aspect for easy file addition and code modification that does not involve having to deal with CLI or changing code files.

### Issues Solved

This platform will aim to ensure code commenting throughout files is up-to-date, sufficient and accurate whilst just looking at the comments and nothing else for quick and swift workflow. Assumes the person using the comment knows the code file therefore does not need to look at the code to determine comment accuracy and other characteristics.

Will hopefully speed up development of larger multi-language complex project.

## Tech Stack

After some research into developing this application I decided to stick with C++ and for a GUI library to go with 'Qt' as it offers great tools suited for this sort of application. Python would also be suitable but development speed is not a major factor and I prefer C++.
`QFileDialog` for selecting files and `QTextStream` for line-by-line file reading. Filter each file line using Regex to extract all the comments from it. Afterwards they can then be displayed utilising `QTableWidget` to display the comments and their line numbers. And then save the file preserving all the structure.