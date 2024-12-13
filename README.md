# PDFToC
---
* Bookmarks editor for PDF files to build a Table Of Contents.
* Need to have [`pdftk`](https://www.pdflabs.com/tools/pdftk-the-pdf-toolkit/) installed.
---
Here is how you can add bookmarks in an existing pdf. I assume your pdf is input.pdf. The info file will be named filename.info, but this name does not matter.

## Step 1: Use pdftk to dump the current meta-data

   pdftk 'input.pdf' dump_data output 'filename.info'

## Step 2: Edit filename.info
For each bookmark, add at the top of the file the following lines:

   BookmarkBegin
	BookmarkTitle: name
	BookmarkLevel: level
	BookmarkPageNumber: page number


* name is the name of the bookmark
* level starts at 1. Every new level remains the same, increases by 1 or goes back to a previous level in the file.
* page number is the number of the page you will go to when you click on the bookmark.

The page number is needed, but only used by pdftk to create the link to the page with that number. The page numbers do not have to be in increasing order. For example, even if the index is at the end of the pdf, you can put the corresponding bookmark just after the cover page.

## Step 3: Update the meta-data:

   pdftk 'input.pdf' update_info 'filename.info' output 'output.pdf'


Source [askubuntu](https://askubuntu.com/questions/1264322/creating-table-of-contents-outline-in-the-pdf-and-link-to-its-pages).
