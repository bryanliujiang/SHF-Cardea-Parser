# **SHF Cardea Parser Manual**

---

## **Table of Contents**
1) Overview
1) Directions
1) Parameters
1) Functions and Assets
1) Advanced Setup
1) Troubleshooting
1) Other Resources

---

## **Overview**
This manual was written to be read and understood by anybody regardless of 
their background in programming. However, technical terms may be present at 
times for sake of completeness/conciseness. These can simply be ignored if they 
are not applicable to one's use case. One should not have to be familiar 
with any technical terms to still understand the program or troubleshoot a 
problem.

This program consists of three main stages:

1) Sanitation stage
2) Cardea-compatible conversion stage
3) Duplicate action stage

### **Sanitation Stage**

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
delimiter that can be user specified ('$' character by default) and is not 
expected to appear within any entry. Once the document is sanitized, it can be 
manipulated freely without worry of confusion between characters within 
entries and as delimiters.

### **Cardea-Compatible Conversion Stage**

Cardea requires a very specific layout of the file it accepts. While patient 
information is processed in a separate SHF server which then generates a CSV 
file storing information that Cardea needs, it does not produce a Cardea-
compatible file. This is the motive for the conversion stage (and this program 
itself).

In this stage, the actual conversion of the raw input CSV file to a Cardea-
compatible output file takes place. Instead of starting from the input file 
and taking away/modifying its entries, this program actually builds the 
output file from scratch, while importing any appropriate entries from the 
input as necessary. Other pertinent processing is also carried out during this 
stage, such as determining if a patient's consent form is on file or handling 
forms filled out in different languages, as some examples. Additionally, 
problematic entries are corrected or pointed out by this program, such as 
removing extraneous spaces from entries (Cardea does not handle extra spaces 
well) and warning when fields were left empty. Once the document is parsed, 
it must now be handled differently from the raw input CSV file, as the new 
layout would be drastically altered from the original.

### **Duplicate Action Stage**

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
duplicate is automatically removed by the program, while only a warning is 
given if the less strict criteria are met. This program is also configured 
to detect and warn if patient first and last names are swapped, which might 
indicate a possible duplicate. Defining these criteria cannot be done through 
the program interface but must require editing the source code itself, which 
is explained later in the manual (see Functions and Assets; 
`track_duplicates_including_this()`). Once duplicates in the document are 
handled, a finalized output file is produced and ready to be accepted by 
Cardea.

---

## **Directions**
1) Place the raw input CSV file in the same location as the parser program.
1) Launch the program and follow the prompts.
1) Before console closes, an event log should be printed as well as a prompt 
   to exit the program.
1) The Cardea-compatible output should be in the same location as 
   the program along with the log file

	 **NOTE:** If you run the program with the same event name, the event logs
             will be appended to (not write over) the existing log file.

---

## **Parameters**
Immediately below is a list of all the user-adjustable parameters, their 
default values, and their C++ data types, separated by colons. Ignore the 
quotation marks. Each parameter will be described in detail afterward.

\* Parameters marked with the asterisk can only be initialized in the custom 
version of the program (see Advanced Setup section).

<br>

One can freely initialize any of the following parameters without worry:

**NAME_INPUT** : "inputForCardea" : `const std::string`

**NAME_OUTPUT** : "outputForCardea" : `const std::string`

**NAME_EVENT** : "screeningName" : `const std::string`

**NAME_FORM_PATH** : "C:\\Users\\Bryan\\SHF\\Heart Screenings\\Forms" : 
`const std::string`

\***FORM_YES** : "Yes" : `const std::string`

\***FORM_NO** : "" : `const std::string`

<br>

One should be more wary about initializing the following parameters:

\***NAME_LOG** : "log_" + **NAME_EVENT** + ".txt" : `const std::string`

\***NAME_INPUT_CLEAN** : "sanitized.csv" : `const std::string`

\***NAME_OUTPUT_DUPLICATES** : "duplicates.csv" : `const std::string`

\***NAME_ENGLISH** : "English" : `const std::string`

\***DELIMITER_CLEAN** : '$' : `const char`

\***DELIMITER_CSV** : ',' : `const char`

<br>

#### **NAME_INPUT**
**Default:** 

"inputForCardea"

**Data Type:**

`const std::string`

**Description:** 

The name of the raw input CSV file to convert into a Cardea-compatible format. 

**Notes:**

The program will automatically append the ".csv" extension to the name.

<br>

#### **NAME_OUTPUT**
**Default:** 

"outputForCardea"

**Data Type:**

`const std::string`

**Description:** 

The desired name for the final Cardea-compatible output file. 

**Notes:**

The program will automatically append the ".csv" extension to the name.

<br>

#### **NAME_EVENT**
**Default:** 

"screeningName"

**Data Type:**

`const std::string`

**Description:** 

The name of the current screening event based on the naming of the consent 
form files. 

**Notes:**

This program will automatically check if patients have already 
filled out consent forms by scanning the files labeled with this particular 
event name. 

<br>

#### **NAME_FORM_PATH**
**Default:** 

"C:\\Users\\Bryan\\SHF\\Heart Screenings\\Forms"

**Data Type:**

`const std::string`

**Description:** 

The absolute path to the directory/folder containing the patient consent forms. 

**Notes:**

Although Windows paths use backslash '\\' as the separator, this program 
will accept forward slashes '/' as well. This program will not proceed 
until a valid path is given.

<br>

#### **FORM_YES**\*
**Default:** 

"Yes"

**Data Type:**

`const std::string`

**Description:** 

The text to place into cells of the Consent column of the Cardea-compatible 
output file when the consent form of a patient is found in the path 
**NAME_FORM_PATH**.

**Notes:**

This parameter was included for sake of user control if it ever need be 
modified in the future. In the meantime, it likely never needs to be 
changed from the default.

<br>

#### **FORM_NO**\*
**Default:** 

""

**Data Type:**

`const std::string`

**Description:** 

The text to place into cells of the Consent column of the Cardea-compatible 
output file when the consent form of a patient is NOT found in the path 
**NAME_FORM_PATH**.

**Notes:**

This parameter was included for sake of user control if it ever need be 
modified in the future. In the meantime, it likely never needs to be 
changed from the default (which is empty).

<br>

#### **NAME_LOG**\*
**Default:** 

"log_” + **NAME_EVENT** + “.txt"

**Data Type:**

`const std::string`

**Description:** 

The desired name for the log file detailing the events and set parameters 
from previous runs of this program.

**Notes:**

If this program is run more than once for the same event name initialized to 
**NAME_EVENT**, the respective log elements will be appended to the same 
log file. In other words, this program will never erase previously created 
logs, only add on to them. The most recent logs will be at the bottom of the 
log file.

If the source code is ever modified such that the initialization order of 
parameters is altered, note that the initialization of this parameter depends 
the prior initialization of **NAME_EVENT** and thus should come after it. 
This also means that if a typo was made in the initialization of 
**NAME_EVENT**, a whole separate log file would be created with that typo in 
its name.

<br>

#### **NAME_INPUT_CLEAN**\*
**Default:** 

"sanitized.csv"

**Data Type:**

`const std::string`

**Description:** 

The name for the intermediate file produced by this program following the 
sanitation stage.

**Notes:**

Note that the ".csv" extension is not automatically applied to this parameter. 
There is nothing inherently special about the default name of this parameter; 
the name was chosen to be descriptive. This file is automatically deleted by 
this program.

<br>

#### **NAME_OUTPUT_DUPLICATES**\*
**Default:** 

"duplicates.csv"

**Data Type:**

`const std::string`

**Description:** 

The name for the intermediate file produced by this program following the 
Cardea-compatible conversion stage.

**Notes:**

Note that the ".csv" extension is not automatically applied to this parameter. 
There is nothing inherently special about the default name of this parameter; 
the name was chosen to be descriptive. This file is automatically deleted by 
this program.

<br>

#### **NAME_ENGLISH**\*
**Default:** 

"English"

**Data Type:**

`const std::string`

**Description:** 

The exact text from the the raw input CSV file indicating that a specific form 
was filled out by a patient in English.

**Notes:**

Incorrect initialization of this parameter may result in significant portions 
of form data being missing in the final Cardea-compatible output file.

This parameter was included for sake of user control if it ever need be 
modified in the future. In the meantime, it likely never needs to be 
changed from the default.

<br>

#### **DELIMITER_CLEAN**\*
**Default:** 

'$'

**Data Type:**

`const char`

**Description:** 

The single character to designate as the sanitized delimiter.

**Notes:**

This should be the singular character to forbid from being entered when 
patients submit forms as it is heavily relied upon in this program for 
sanitation and duplicate tracking. Incorrect initialization of this parameter 
may result in a significantly disfigured output file that would be 
incompatible with Cardea, if not crash this program. 

If the default character is anticipated to be present, this parameter can be 
modified as to represent an alternative character. Otherwise, it never needs 
to be changed from the default.

<br>

#### **DELIMITER_CSV**\*
**Default:** 

','

**Data Type:**

`const char`

**Description:** 

The character used as the delimiter in the raw input CSV file.

**Notes:**

This parameter was included for sake of user control if it ever need be 
modified in the future. Although most CSV files use commas as the delimiter, 
it is possible that the raw input CSV file may use a different delimiter, 
warranting this parameter to be modified. In the meantime, however, it likely 
never needs to be changed from the default.

<br>









/* Cannot initialize the following parameters since source code might have to be adjusted */

const std::string NAME_FORM = "SHF-Consent_ _ _ _ -SIGNED.pdf"; // consent form file name, adding a whitespace where a variable is
const int NUM_LANG_FIELDS = 10; // number of columns unique to a language
const std::vector<std::string> HEADERS = // names and order of headers for output csv
{

---

## **Functions and Assets**



<!--
CSV stands for comma-separated values. 

Unfortunately, the SHF server cannot completely 
catch and correct all the issues from the patient information it receives. 

or if a 
phone number is valid

However, the quality of the content in the output file is only as good as that 
of the input file.

Sometimes fields are left empty, sometimes an entry was 
formatted incorrectly (leaving extra spaces, for example, which Cardea does 
not handle well), and other times patients submit multiple times, leading to 
duplicate patient information that can lead to confusion in identifying 
patients during screenings.
-->

---

## **Advanced Setup**



---

## **Troubleshooting**

---

## **Other Resources**
<!--
```
/* Parser functions used in the program */
void Parse(); // runs the program workflow: Sanitize() -> MakeCardeaCompatible() -> RemoveDuplicatesFrom()
void Sanitize(std::ifstream& input); // sanitizes input and produces sanitized output file for processing
void MakeCardeaCompatible(std::ifstream& input); // parses sanitized input according to Cardea, produces proper output for Cardea
void RemoveDuplicatesFrom(std::ifstream& input); // produces output for Cardea without duplicates and/or with warnings of them
void CreateLogFile(); // appends to log file significant program events

/* Assets intended for MakeCardeaCompatible() */
void remove_spaces_from_this(std::string& entry); // any entry given will have its whitespaces removed
bool file_exists(std::string path, std::string file); // tells whether file from given path exists
std::string ten_digit_phone_number(std::string digits); // formats raw ten digits into a phone number compatible with Cardea
std::string consent_form_file_name(std::string screeningName, std::string ID,
    std::string LN, std::string FN, std::string format); // generates consent form file name based on the NAME_FORM format

/* Assets intended for RemoveDuplicatesFrom() */
void track_duplicates_including_this(std::string entry, bool to_warn = true, bool for_swap = false, bool to_remove = false); // specifies handling duplicates
std::string swapped(std::string potential_duplicate); // swaps first two entries of columns tracked for swaps
std::string potential_duplicate_to_warn; // string representation of entries tracked for duplicates to warn of
std::string potential_duplicate_to_remove; // string representation of a row of entries tracked for duplicates to remove
std::string potential_swap; // string representation of a row of entries tracked for swaps
std::unordered_set<std::string> rows_scanned_for_duplicates; // holds strings to check against for duplicates to remove and/or warn of
std::unordered_set<std::string> rows_scanned_for_swaps; // holds strings to check against for swaps
std::unordered_multiset<std::string> list_of_duplicates; // holds strings of confirmed duplicates to remove and/or warn of
std::unordered_multiset<std::string> list_of_swaps; // holds strings of confirmed swaps
std::unordered_map<std::string, std::vector<int>> list_of_duplicates_found; // holds locations of rows involved with duplicates
std::unordered_map<std::string, std::vector<int>> list_of_swaps_found; // holds locations of rows involved with swaps

/* Assets for CreateLogFile() */
std::string add_log(std::string log); // entry given will be appended to the log file; returns log message itself
std::vector<std::string> logs; // holds log entries
```
```

```
-->