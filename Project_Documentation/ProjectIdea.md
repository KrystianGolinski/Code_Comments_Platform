## Code Comments Platform

This platform will allow the upload of supported code files (.cpp, .h, .py, .ts ...) and output all comments within each file, their line numbers and the ability to edit the comments and save the file with the altered comment structure. The platform should feature a GUI aspect for easy file addition and code modification that does not involve having to deal with CLI or changing code files.

### Issues Solved

This platform will aim to ensure code commenting throughout files is up-to-date, sufficient and accurate whilst just looking at the comments and nothing else for quick and swift workflow. Assumes the person using the comment knows the code file therefore does not need to look at the code to determine comment accuracy and other characteristics.

Will hopefully speed up development of larger multi-language complex project.


## Notes

Plan on using this platform as a helper to help me develop my other platform which primarily uses C++, Python, React, TypeScript and some others. So that is the main end goal but language support will be added iteratively. 

Not planned to support docstrings as I haven't used them in my code, but maybe at some point it could be revisited.

The editing would be in place, the files would be uploaded then comments would be extracted and then I could just change the comments in the GUI and then click on 'Save' (or similar) and the file is updated (the opened file that is) and I can move on with my day. I'd expect the file formatting and indentation etc to stay the same. It should work same as: Open VS Code, Open file, edit a comment, press Shift+S (save), quit VS Code. But with my platform I wouldn't have to deal with color syntax highlights or seeing any of the bulky code.

For the GUI the (original) line numbers should be shown, if the edited comment is really long I don't think it's needed to update the line numbers. I mostly just want to use this as a rough idea of 'Is this comment and the next comment next to each other or universes apart and to be treated as separate identities'. 

The planned GUI is linux-based desktop app (can be as simple as a C++ executible or something) just something that I can add files to without having to manually change the script each time or dealing with CLI really. Nothing impressive, straight functionality. 

Should support batch uploads at some point of the dev cycle, as previously mentioned the comments should just be printed out with no highlights. No original code to be shown either just main focus on comments written in normal text.

The results should just feature each comment in a table-like structure {Line | Comment}

Just a simple 'Edit comments of a code file and no more' app.

## Tech Stack

After some research into developing this application I decided to stick with C++ and for a GUI library to go with 'Qt' as it offers great tools suited for this sort of application. Python would also be suitable but development speed is not a major factor and I prefer C++.
`QFileDialog` for selecting files and `QTextStream` for line-by-line file reading. Filter each file line using Regex to extract all the comments from it. Afterwards they can then be displayed utilising `QTableWidget` to display the comments and their line numbers. And then save the file preserving all the structure somehow.