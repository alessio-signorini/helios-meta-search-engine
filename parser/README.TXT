HOW TO WRITE A PARSER SCRIPT ---------------------------------------------------
Altought  write  a  parser script is really simple, and most of the instructions
can  be found in the source code, this quick guide will help you to learn easily
how to do it.

                                                   <alessio-signorini@uiowa.edu> 



COMMANDS -----------------------------------------------------------------------

   l <string>

Search  for  <string>  in the data downloaded so far, and move the cursor to the
beginning of the string once found.


   + <n>

Move the cursor forward of <n> characters.


   - <n>

Move the cursor backward of <n> characters.


   g <info>

Initialize the buffer were the next relevant informations will be stored. It can
assume the value TITLE, URL, or TEXT.


   u <string>

Capture  from  the  point in which the command "g" was used, to the beginning of
<string>,  saving  the text in the buffer previously pointed in the command "g".
Then the cursor is moved to the end of <string>.


   c
	
Clean the last information extracted from HTML tags and characters.


   i <n> <string>

Simple  IF  command.  If  <string>  is found within <n> characters skip the next
instruction, otherwise execute it.


   j <line>
	
Jump to <line>. Keep in mind that the first line is 0.


   p

Print  on-screen the data captured so far. This command reset the data contained
in the buffers once printed.


   q

Stop  to  consider  interesting this engine. Close the socket, and don't look at
the buffer anymore.