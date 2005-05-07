HOW TO ADD A NEW ENGINE --------------------------------------------------------
Add  a  new  engine is very simple. First, you have to create a new entry in the
file  ENGINES.DAT  in  the  main  directory of the software. Then, you create an
empty  file  with  the  name  of the engine and with the extention ".prs" in the
"/parser" directory.

Now launch the software as

   helios -d -p <query> <newEngineName>

this  will  make  HELIOS  query  the  new search engine, without considering the
parser  script  (-p) and duping (-d) on the hard disk the data received. Now you
can go in the parser scripts directory, and create the ad-hoc parser script.


 
NEW ENTRY IN THE FILE ENGINES.DAT ----------------------------------------------
The new entry should look like

   [newengine]
   host					=	216.239.59.104
   port					=	80
   desidered_pages	=	1
   result_per_page	=	100
   next_page_calc		=	2
   {
   	GET /search?q=%QUERYSTR&num=%RESULTS&start=%PAGE HTTP/1.0
   	HOST: www.newengine.com

   }

Between  the  square  brakets there is the name of the new engine. In <host> you
write  the  address of the engine, that can either be in dot-format or as domain
name  (slower).  The  parameter <port> is self explanatory. In <desidered_pages>
you  write  the default number of pages that you desire to download sequentially
from the new engine, and in <result_per_page> the default number of results that
you want in each page.

This  two  last parameters will be used if HELIOS will be launched with the name
of the engine, without any specific parameter. As

   helios flowers newengine

they can be overrided with the syntax

   helios flowers newengine=<r>,<p>,<s>

where  <r>  determine  the  number of results in each page, <p> is the number of
sequential pages to download and <s> is the first page to download. These values
will  be  substituted  to  %RESULTS  and  %PAGE.  The  query,  obviously will be
substituted to %QUERYSTR.

The  field  <next_page_calc>  determine how the number of the page is handled by
the  search  engine.  If I desire page 5, for example, some search engine accept
the  number  5  as  it is, while others use the number of results (like 50, if I
requested  10 results for each page), and so on. The various scheme supported by
HELIOS are

   0) %PAGE = page
   1) %PAGE = page + 1
   2) %PAGE = page * rpp
   3) %PAGE = (page * rpp) + 1
	
and should embrace all the possibilities.

The curling brackets contain the HTTP request, with the variables. You can write
whatever you want here, but remember to leave an empty line at the end.

The  file  ENGINES.DAT  has  to  be  terminated with an empty line. Comments are
defined by the character # and skipped by the system.
