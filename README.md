# **Overview**
This program was intentionally not written in the most efficient way, but was 
written to prioritize portability (all components of this program resides 
in one file containing the source code) and beginner-friendliness so that 
one would require minimal training to be able to maintain this program. For 
example, the amount of global variables (typically discouraged when writing 
a program) could have been minimized, but the decision was made to keep them 
for sake of simplifying/making easier to read the source code. Although the 
source code of this program may not be the best reference for general 
programming strategies, it better serves as an educational asset for anyone 
with basic programming knowledge wishing to learn the C++ programming language 
itself, as a diverse set of C++ concepts have been included in the 
implementation of this program along with their explanations throughout 
this manual and the source code itself.

This program was written in C++ and compiled in Windows 10 Visual Studio 2019, 
using (and requiring at the earliest) the ISO C++17 Standard. This program is 
intended for the Windows 10 operating system.

ASCII text art was generated from or adapted from 
<https://patorjk.com/software/taag/>. The Saving Hearts Foundation logo was 
adapted from <http://loveascii.com/hearts.html>.

Fundamentally, this program consists of three main stages:

1) Sanitation stage

2) Cardea-compatible conversion stage

3) Duplicate action stage

## **Sanitation Stage**

CSV stands for comma-separated values. As the name implies, individual entries 
from spreadsheets, like the raw input CSV file for this program, are commonly 
separated by commas when the file is in the CSV format. This separator 
character (the comma in this case) is known as a "delimiter." 

While there is nothing inherently wrong with commas as delimiters, it does 
become problematic when one wishes to parse through a comma-delimited 
document. The reason is that it is not uncommon that the entries delimited by 
commas contain these very commas themselves. This complicates the parsing 
process, as one must then keep track of which commas are true delimiters 
and which are just part of the entries. This is the motive for the 
sanitation stage.

In this stage, this program "sanitizes" the raw input CSV file by scanning 
the whole document for the comma delimiter (can be other characters also, see 
Advanced Setup), ignoring commas deemed to be from entries and replacing 
commas deemed to be delimiters with an alternative character, a "clean" 
delimiter that can be user-specified ('$' character by default, see Program 
Parameters in the manual; **DELIMITER_CLEAN**) and is not expected to appear 
within any entry. Once the document is sanitized, it can be manipulated freely 
without worry of confusion between characters within entries and as delimiters.

## **Cardea-Compatible Conversion Stage**

Cardea requires a very specific layout of the file it accepts. While patient 
information is processed in a separate SHF server which then generates a CSV 
file storing information that Cardea needs, it does not produce a 
Cardea-compatible file. This is the motive for the conversion stage (and 
this program itself).

In this stage, the actual conversion of the raw input CSV file to a 
Cardea-compatible output file takes place. Instead of starting from the input 
file and taking away/modifying its entries, this program actually builds the 
output file from scratch, while importing any appropriate entries from the 
input as necessary. Other pertinent processing is also carried out during this 
stage, such as determining if a patient's consent form is on file or handling 
forms filled out in different languages. Additionally, problematic entries 
are corrected or pointed out by this program, such as removing extraneous 
spaces from entries (Cardea does not handle extra spaces well) and warning 
when fields were left empty. Once the document is parsed, it must now be 
handled differently from the raw input CSV file, as the new layout would be 
drastically altered from the original.

## **Duplicate Action Stage**

After the conversion stage, the file can now technically be read by Cardea. 
However, it is possible that patients submit their information multiple times 
for the same screening (such as to update older information or due to network 
error), resulting in duplicated patient information that can lead to confusion 
in properly identifying patients during screenings. Unfortunately, the SHF 
server does not check for duplicate submissions. This is the motive for the 
duplicate action stage.

In this stage, this program detects and takes action against duplicates, 
based on a pre-determined configuration of what qualifies as a duplicate. 
There are two levels of strictness; if the stricter criteria are met, the 
duplicate is automatically removed by this program, while only a warning is 
given if the less strict criteria are met. This program is also configured 
to detect and warn if patient first and last names are swapped, which might 
indicate a possible duplicate. Defining these criteria cannot be done through 
the program interface but must require editing the source code itself, which 
is explained in the manual (see Functions and Assets in the manual; 
`track_duplicates_including_this()`). Once duplicates in the document are 
handled, a finalized output file is produced and ready to be accepted by 
Cardea.

---

# **Directions**
1) Place the raw input CSV file in the same location as this program.

1) Launch this program and follow the prompts. 

   To follow this step an alternative way, see Advanced Setup; 
   Accelerated Parameter Initialization.

1) Before the console closes, an event log should be listed, followed by a 
prompt to exit this program.
   
1) The Cardea-compatible output should be in the same location as this program 
along with the log file.

    **NOTE:** If one runs this program with the same event name, the event 
    logs will be appended to (not write over) the existing log file.

---

# **Advanced Setup**

Below are some alternative ways to run this program.

<br>

## **Modes**

When this program is launched, a greeting is first displayed followed by a 
prompt to press ENTER to start this program. Instead of just pressing ENTER, 
one can instead enter in a specific keyword to run this program in an 
alternative mode. This program does not continue until a valid keyword is 
entered.

This whole process is dictated by the initialization function `init_MODE()`. 
The mode is stored in its corresponding constant global variable called 
**MODE**. This should be the very first variable to initialize, even before 
any of the program parameters (see Program Parameters in the manual).

The modes that can be run are described below. Ignore the quotation 
marks.

**NOTE:** Capitalization does not matter.

<br>

### **Express Mode**

**Description:**

Advanced parameters are automatically initialized to their program defaults.

This mode is implicitly set by default if no keyword is entered at the 
beginning of this program.

**Keyword:**

"EXPRESS"

**Notes:**

Having an explicit keyword for this mode is helpful when running this program 
from a text file containing the pre-initialized program parameters (see 
Advanced Setup section; Accelerated Parameter Initialization) since it allows for 
visible indication that the default of this program is to be run instead of 
just having a blank line at the top of the document. 

<br>

### **Custom Mode**

**Description:**

Gives access to initializing the advanced parameters from the program 
interface. The program-parameter name will also be displayed beside each 
initialization prompt.

**Keyword:**

"CUSTOM"

**Notes:**

See Program Parameters in the manual; Custom-Only (Advanced) Parameters 
to understand the implications of changing a particular advanced 
parameter from its default value.

<br>

### **Retain Mode**

**Description:**

Gives access to initializing the advanced parameters from the program 
interface (same as Custom Mode) but also retains the intermediate files 
produced by this program after the sanitation and Cardea-compatible conversion 
stages. The program-parameter name will also be displayed beside each 
initialization prompt.

**Keyword:**

"RETAIN"

**Notes:**

Normally, these intermediate files are deleted automatically by the program 
by the function `Parse()`. This mode prevents that from happening. This mode 
might be useful for debugging purposes. 

<br>

### **Other Keywords**

Here are some Easter eggs to reward you for reading this manual. Although not 
modes per se, try entering any of the following keywords and see what happens!

- "HEART"
- "SHF"

<br>

## **Accelerated Parameter Initialization**

While one can manually initialize each parameter line by line, this process 
can be accelerated by pre-initializing each expected parameter in a 
separate text document and then copy and pasting that into this program.

1) Create an empty text document. 

    This can be done by opening any desired location in File Explorer, 
    right-clicking in any empty area of that location, and selecting "New" 
    and then the "Text Document" option. Give the text document any name, 
    such as "parameters.txt" (".txt" might already be appended).
    
1) Enter into the document the desired mode to run this program in (see 
Advanced Setup section; Modes). Press ENTER once to move to the next line.

    **TIP:** For readability, enter the mode keyword in all caps. That way, 
    one can recognize the mode and distinguish it from the parameter 
    initializations from a glance of the document.

1) Enter into the document line by line the initialization of each parameter 
in the order as if running this program itself. 

    Press ENTER only once after each line, including after the final line. If 
    one wishes to use the default value of a parameter (see Program 
    Parameters), simply leave that line empty.
    
    **TIP:** For readability, type "EXIT" on the line just after the final 
    line instead of just leaving that line blank. That way, one can recognize 
    when this program would exit from a glance of the document.


1) If one would like to have this program exit immediately upon completion and 
skip seeing the event logs printed to the console altogether, add one 
additional empty line to the end of the document (simulates pressing ENTER 
to exit the program). The event logs are still appended to the log file.

1) Select all the contents of the document (Ctrl+A), copy its contents 
(Ctrl+C), run this program, and then simply paste the contents into the 
console (Ctrl+V).

This program interprets the end of each line as if ENTER were pressed and 
initializes each parameter with the contents of each line in the same 
order.

One advantage to this approach is that it allows for fast runs and reruns of 
this program. This is especially true if this program has to be run or rerun 
at a later time when one may not immediately recall how to initialize the 
parameters. 

---

# **Troubleshooting**

**NOTE:** Some solutions may require running the custom mode of this 
program to initialize more advanced parameters (see Advanced Setup section; 
Custom Mode).

Below are some of the issues that may arise from this program. If an issue is 
not addressed, report it to the SHF tech administrator. All these suggestions 
are assuming this program was compiled or run on a computer using Windows 10.

<br>

## **[ ERROR ] ... could not be opened or does not exist.**

First, be sure the input CSV is in the same folder as this program. If so, 
make sure there are no typos when inputting the input CSV name (case matters). 
Remember that ".csv" is automatically applied to the input name by this 
program, so be sure that the input name does not redundantly include ".csv" 
when inputting the input CSV name (input "inputForCardea" instead of 
"inputForCardea.csv").

If the error persists, this program might be placed in an access-restricted 
location. Try moving this program to another location (such as Desktop). If 
the error still persists, then the input CSV itself might have access 
restrictions. Contact the administrator if this is the case. Another possible 
reason is that this program itself might have been access-restricted upon 
installation (possibly by antivirus software). Try granting exceptions to 
this program.

<br>

## **[ ERROR ] Could not open sanitized input file.**

Make sure no other file in the same folder as this program has the same name 
as whatever the parameter **NAME_INPUT_CLEAN** was initialized to.

Otherwise, this error occurred most likely due to the program being placed in 
an access-restricted location. Try moving this program to another location 
(such as Desktop). If the error persists, this program itself might have been 
access-restricted upon installation (possibly by antivirus software). Try 
granting exceptions to this program.

<br>

## **[ ERROR ] Could not open output with duplicates file.**

Make sure no other file in the same folder as this program has the same name 
as whatever the parameter **NAME_OUTPUT_DUPLICATES** was initialized to. Also, 
be sure that the parameter **NAME_INPUT_CLEAN** was not initialized with the 
same name as **NAME_OUTPUT_DUPLICATES**.

<br>

## **The final Cardea-compatible output file is not appearing.**

If the log file exists, check the most recent log events for any errors 
(most recent logs are appended to the bottom of the file). If no error is 
present, check that the parameter **NAME_OUTPUT** is not initialized with 
the same names as the parameters **NAME_INPUT_CLEAN** or 
**NAME_OUTPUT_DUPLICATES**.

If the log file does not exist, run this program line by line to check the 
log events for any errors that are printed on the console before the program 
exits. If no error is present, check that the parameter **NAME_OUTPUT** is not 
initialized with the same names as the parameters **NAME_INPUT_CLEAN** or 
**NAME_OUTPUT_DUPLICATES**. Also, be sure this is the case for parameter
**NAME_LOG**.

If none of these solutions work, contact the administrator.

<br>

## **Significant portions of the final output file are blank.**

If the final output file is completely blank, be sure that the raw input CSV 
file is also not empty or that parameter **NAME_INPUT** is initialized to the 
correct file name. Also, be sure that parameter **NAME_OUTPUT** is not 
initialized with the same name as **NAME_INPUT_CLEAN**.

If the final output file is partially blank, check that parameter 
**NAME_ENGLISH** is initialized properly and consistent with what the raw 
input CSV file requires.

<br>

## **The final output file is disfigured.**

Be sure that the parameters **DELIMITER_CLEAN** and **DELIMITER_CSV** are 
initialized properly. This means making sure that the character for 
**DELIMITER_CLEAN** was not used anywhere in the raw input CSV file (a 
patient may have used and submitted it). If so, initialize **DELIMITER_CLEAN** 
to an alternative character not present in the raw input CSV file (see 
Advanced Setup section; Custom Mode). Otherwise, check that the raw input 
CSV file is delimited by the character set by **DELIMITER_CSV** and 
initialize accordingly. One way to check is to open the raw input CSV file 
with a text editor like Notepad (can be done by right-clicking on the file 
and selecting "Open with" or by temporarily changing the file extension from 
".csv" to ".txt" and opening that file again).

If none of these solutions work, contact the administrator, as the SHF server 
may have undergone a formatting change.

<br>

## **Path name is invalid.**

If on Windows, be sure that any folders that have spaces in its name when 
entered in the path are not surrounded by quotation marks.

**TIP:** To be absolutely sure that the path entered is formatted correctly, 
try copy and pasting the location listed in the folder properties. This can be 
done by right-clicking on the folder containing the consent forms and 
selecting the "Properties" option. Under the "General" tab should be a listing 
named "Location:" followed by the path to copy and paste. If the path is long, 
make sure to copy and paste the whole path, as some parts may be cut off from 
view.

---

# **Other Resources**

Be sure to have the updated contact information of the SHF tech administrator!

One may also contact Bryan Jiang by email at <bryanjiang@ucla.edu>.
