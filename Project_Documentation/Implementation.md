# Code Comments Platform - Implementation Details

The Code Comments Platform is a Qt6-based desktop application that extracts comments from source code files, presents them in an editable table interface, and saves modifications back to the original files while preserving code structure.

## Core Architecture

### 1. Comment Extraction (`CommentExtractor`)
- **Purpose**: Parse source code files and identify comment patterns
- **Supported Languages**: C++ (`//`, `/* */`), Python (`#`)
- **Key Insight**: Comments are grouped by proximity - consecutive comment lines form logical groups
- **Data Structure**: `CommentGroup` contains:
  - `lineNumbers`: Original line positions in source file
  - `comments`: Extracted comment text
  - `fullLines`: Complete line context for inline comments
  - `isInline`: Flags to distinguish standalone vs inline comments

### 2. User Interface (`MainWindow`)
- **Layout Strategy**: Scroll area containing dynamically sized tables
- **Table Structure**: One table per source file, one row per comment group
- **Column Design**:
  - Line column: Narrow, right-aligned, shows line numbers vertically
  - Comment column: Stretched, shows grouped comments for editing
- **Intelligent Sizing**: Content height vs window height determines expansion behavior:
  - Large content (>60% window): Natural Qt expansion with scrollbars
  - Small content (≤60% window): Constrained size with bottom spacer

### 3. Multi-line Comment Editing
- **Challenge**: Users can expand single comments into multiple lines
- **Solution**: Mathematical encoding for new line insertion positions
- **Encoding Scheme**: `-(baseLine * 1000 + offset + 1)` for insertions after existing lines
- **Processing Logic**: 
  1. Separate replacements (positive line numbers) from insertions (negative)
  2. Process replacements first to update existing lines
  3. Process insertions sequentially with position adjustment for previous insertions

### 4. File Persistence (`CommentSaver`)
- **Safe Writing**: Use temporary files to prevent data loss
- **Structure Preservation**: Maintain original file formatting and spacing
- **Comment Integration**: Replace existing comments and insert new ones at calculated positions

## Key Design Decisions

### Comment Grouping Logic
Comments are grouped when they appear on consecutive lines, treating them as logical units. This reflects how developers naturally organize related comments and reduces UI clutter.

### Line Number Display
Instead of comma-separated ranges (`1,2,3,4,5`), line numbers are displayed vertically to create visual alignment with their corresponding comment lines, improving readability.

### Dynamic Layout System
The application adapts to content size:
- **Principle**: Don't waste space for small content, but allow full expansion for large content
- **Implementation**: Calculate content height vs available window space ratio
- **Threshold**: 60% ratio determines between constrained and expanded modes

### Multi-line Insertion Algorithm
The mathematical encoding system allows precise insertion tracking:
- **Why**: Traditional line number tracking breaks when multiple insertions occur
- **How**: Encode insertion point and offset in negative numbers
- **Benefit**: Supports complex editing scenarios like expanding comments into multiple paragraphs

## Data Flow

1. **File Loading**: `CommentExtractor` parses files → creates `CommentGroup` objects
2. **UI Population**: Groups populate table rows with formatted display
3. **User Editing**: Multi-line text editor allows comment modification
4. **Change Tracking**: Modified text is parsed back to individual comment lines
5. **File Writing**: `CommentSaver` applies changes while preserving file structure

## Error Handling & Edge Cases

- **Empty Files**: Gracefully handle files with no comments
- **Mixed Comment Types**: Correctly process both inline and standalone comments
- **Large Comment Blocks**: Dynamic sizing prevents UI overflow
- **Concurrent Line Insertions**: Sequential processing with position adjustment
- **File I/O Failures**: Temporary file approach ensures data safety
