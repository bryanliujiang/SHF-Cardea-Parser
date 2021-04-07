/* Ignore; needed to run program properly */
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include <iterator>
#include <filesystem> // requires C++17

/* Ignore; for parameter initialization */
std::string init_NAME_INPUT();
std::string init_NAME_OUTPUT();
std::string init_NAME_EVENT();
std::string init_NAME_FORM_PATH();
std::string init_FORM_YES();
std::string init_FORM_NO();
std::string init_NAME_LOG();
std::string init_NAME_INPUT_CLEAN();
std::string init_NAME_OUTPUT_DUPLICATES();
std::string init_NAME_ENGLISH();
char init_DELIMITER_CLEAN();
char init_DELIMITER_CSV();

/* Ignore; for tracking duplicates */
std::string potential_duplicate_to_warn;
std::string potential_duplicate_to_remove;
std::unordered_set<std::string> rows_scanned;
std::unordered_multiset<std::string> list_of_duplicates;

/* Ignore; for tracking logs */
void CreateLogFile();
std::vector<std::string> logs;

void Parse();

/* Freely adjust any of the following parameters */
const std::string NAME_INPUT = init_NAME_INPUT(); // name of input csv to parse
const std::string NAME_OUTPUT = init_NAME_OUTPUT(); // desired name of parsed output csv for Cardea
const std::string NAME_EVENT = init_NAME_EVENT(); // name of current screening event based on naming of consent form file
const std::string NAME_FORM_PATH = init_NAME_FORM_PATH(); // directory path to the consent forms; any slash works
const std::string FORM_YES = init_FORM_YES(); // what to print into Consent column of output csv if consent form exists
const std::string FORM_NO = init_FORM_NO(); // what to print into Consent column of output csv if consent form does not exist

/* Be more wary about changing the following parameters */
const std::string NAME_LOG = init_NAME_LOG(); // desired name of log file detailing program events
const std::string NAME_INPUT_CLEAN = init_NAME_INPUT_CLEAN(); // ignore; needed to sanitize input properly
const std::string NAME_OUTPUT_DUPLICATES = init_NAME_OUTPUT_DUPLICATES(); // ignore; needed to remove duplicates properly
const std::string NAME_ENGLISH = init_NAME_ENGLISH(); // how it was indicated in the raw input csv that the forms were filled out in English
const char DELIMITER_CLEAN = init_DELIMITER_CLEAN(); // which singular character to forbid while filling out forms; needed to sanitize input properly
const char DELIMITER_CSV = init_DELIMITER_CSV(); // how raw input csv delimits cells; needs to be cleaned out by sanitizer function

/* Changing the following parameters may require code to be edited */
const std::string NAME_FORM = "SHF-Consent_ _ _ _ -SIGNED.pdf"; // consent form file name, adding a whitespace where a variable is
const int NUM_LANG_FIELDS = 10; // number of columns unique to a language
const std::vector<std::string> HEADERS = // names and order of headers for output csv
{
    "MSN",
    "LastName",
    "FirstName",
    "Email",
    "PGNam",
    "PGPhone",
    "Race",
    "Birthdate",
    "Gender",
    "Weight",
    "Height",
    "Sport",
    "OMI",
    "Meds",
    "ExPain",
    "Sync",
    "SOB",
    "Murmur",
    "HiBP",
    "FamHist",
    "SCD",
    "FamDisabled",
    "Consent",
    "Notes"
};

/* Parser functions used in the program */
void Sanitize(std::ifstream& input); // sanitizes input, produces sanitized output
void MakeCardeaCompatible(std::ifstream& input); // parses input according to Cardea, produces proper output for Cardea
void RemoveDuplicatesFrom(std::ifstream& input); // produces out for Cardea without duplicates

/* Other useful functions */
void remove_spaces_from_this(std::string& entry); // any entry given will have its whitespaces removed
void warn_duplicates_including_this(std::string entry); // duplicate must match this entry to be warned of in RemoveDuplicatesFrom()
void remove_duplicates_including_this(std::string entry); // duplicate must match this entry to be removed in RemoveDuplicatesFrom()
bool file_exists(std::string path, std::string file); // tells whether file from given path exists
std::string add_log(std::string log); // entry given will be appended to the log file; returns log message itself
std::string nine_digit_phone_number(std::string digits); // formats raw nine digits into a phone number compatible with Cardea
std::string consent_form_file_name(std::string screeningName, std::string ID,
    std::string LN, std::string FN, std::string format); // generates consent form file name based on the NAME_FORM format

/* Actual program */
int main()
{
    Parse(); // run the parser functions based on configured parameters

    CreateLogFile(); // generate log file of significant events that occurred during the program

    return 0; // end of program
}

/* Function definitions */
void Parse()
{
   ///////////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////// PART 1: SANITIZE RAW INPUT CSV ////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////

   /* Ignore; checks if raw input csv can be opened */
    std::ifstream raw_input(NAME_INPUT);
    if (!raw_input)
    {
        std::cout << "\n\n" + add_log("[ ERROR ] '" + NAME_INPUT + "' could not be opened or "
            + "does not exist.")
            + "\nMake sure the input .csv is in the same folder as this program.\n" << std::endl;
        std::string exit;
        while (exit != "exit")
        {
            std::cout << "You may close the program or type 'exit' to exit." << std::endl;
            std::getline(std::cin, exit);
        }
        return; // terminate program early due to error
    }

    /* Sanitize raw input csv */
    Sanitize(raw_input);

    raw_input.close(); // finished with sanitizing raw input csv

    ///////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////// PART 2: PRODUCE CARDEA OUTPUT CSV //////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /* Ignore; checks if sanitized input file can be opened */
    std::ifstream sanitized_input(NAME_INPUT_CLEAN);
    if (!sanitized_input)
    {
        std::cout << "\n\n" + add_log("[ ERROR ] Could not open sanitized input file.") << std::endl;
        std::string exit;
        while (exit != "exit")
        {
            std::cout << "You may close the program or type 'exit' to exit." << std::endl;
            std::getline(std::cin, exit);
        }
        return; // terminate program early due to error
    }

    /* Parse sanitized input file that was just produced */
    MakeCardeaCompatible(sanitized_input);

    sanitized_input.close(); // finished with parsing sanitized input file
    std::remove(NAME_INPUT_CLEAN.c_str()); // delete the sanitized input file

    ///////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////// PART 3: REMOVE DUPLICATE ENTRIES ///////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /* Ignore; checks if output still with duplicates file can be opened */
    std::ifstream output_with_duplicates(NAME_OUTPUT_DUPLICATES);
    if (!output_with_duplicates)
    {
        std::cout << "\n\n" + add_log("[ ERROR ] Could not open output with duplicates file.") << std::endl;
        std::string exit;
        while (exit != "exit")
        {
            std::cout << "You may close the program or type 'exit' to exit." << std::endl;
            std::getline(std::cin, exit);
        }
        return; // terminate program early due to error
    }

    /* Remove duplicates from output with duplicates file that was just produced */
    RemoveDuplicatesFrom(output_with_duplicates);

    output_with_duplicates.close(); // finished with removing duplicates from file
    std::remove(NAME_OUTPUT_DUPLICATES.c_str()); // delete the output with duplicates file
}

void Sanitize(std::ifstream& input)
{
    /* Generates sanitized input file */
    std::ofstream output;
    output.open(NAME_INPUT_CLEAN);

    /* First parser to sanitize input csv */
    /*
        Adapted from:
        http://www.zedwood.com/article/cpp-csv-parser
        http://www.cplusplus.com/forum/beginner/227841/
    */
    std::string row; // holds current row of raw input csv
    while (std::getline(input, row)) // iterate through every row of raw input csv
    {
        std::stringstream iss(row); // allows row to be broken down into cells separated by dirty delimiter
        std::string cell; // holds and delivers contents of cleaned cell
        bool inquotes = false; // whether contents are inside quotation marks or not
        char c; // wink wink ;)
        while (iss.get(c))
        {
            if (c == '"') // toggles once entering or leaving quotation mark
            {
                inquotes = !inquotes;
            }
            if (!inquotes && c == DELIMITER_CSV) // end of current cell reached
            {
                output << cell << DELIMITER_CLEAN; // deliver clean cell contents; sanitizes input csv for second parser
                cell.clear(); // empty out current cell for the next cell
            }
            else
            {
                cell += c; // hold clean contents for current cell
            }
        }
        output << '\n'; // move to next row
    }

    output.close(); // finished with producing sanitized input csv
}

void MakeCardeaCompatible(std::ifstream& input)
{
    /* Generates proper output file for Cardea (still may contain duplicates) */
    std::ofstream output;
    output.open(NAME_OUTPUT_DUPLICATES);

    /* Second parser to generate Cardea layout output file by iterating through every cell of the sanitized input csv */
    for (auto header : HEADERS)
        output << header << DELIMITER_CLEAN; // adding header row here for sake of consistency
    output << '\n'; // move to next row

    int row_number = 1; // headers was row 1
    std::string row; // holds current row of sanitized input csv
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ignore header row from input csv
    while (std::getline(input, row)) // iterate through every row of sanitized input csv
    {
        row_number++; // increment current row number

        std::istringstream iss(row); // allows row to be broken down into cells separated by clean delimiter
        std::string cell; // holds contents of current cell of current row
        bool inEnglish = false; // checks if language of form is English or not
        std::string MSN, LastName, FirstName; // will be used to check for filled consent forms

        std::getline(iss, cell, DELIMITER_CLEAN); // at first cell of input csv (A)
        // ignore column

        std::getline(iss, cell, DELIMITER_CLEAN); // second cell (B)
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: MSN column of row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // keep cell as is; MSN cell for output
            MSN = cell; // store MSN
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // third cell (C) . . .
        // ignore column

        std::getline(iss, cell, DELIMITER_CLEAN); // D
        // ignore column
        remove_spaces_from_this(cell);
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: Language column of row " + std::to_string(row_number)) << std::endl;
            inEnglish = true; // assume in English anyway
        }
        else if (cell == "English")
        {
            inEnglish = true; // explicitly in English
        }
        else if (cell == "Spanish")
        {
            ; // do nothing for now (inEnglish == false by default)
        }
        else if (cell == "Vietnamese")
        {
            ; // do nothing for now (inEnglish == false by default)
        }
        
        std::getline(iss, cell, DELIMITER_CLEAN); // E
        remove_spaces_from_this(cell); // this cell should not contain any space
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: LastName column of row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // LastName
            LastName = cell; // store last name
        }
        
        std::getline(iss, cell, DELIMITER_CLEAN); // F
        remove_spaces_from_this(cell);
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: FirstName column of row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // FirstName
            FirstName = cell; // store first name
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // G
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: Email column of row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // Email
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // H
        // ignore column

        std::getline(iss, cell, DELIMITER_CLEAN); // I
        // ignore column

        std::getline(iss, cell, DELIMITER_CLEAN); // J
        remove_spaces_from_this(cell);
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: PGNam column of row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // PGNam
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // K
        // ignore column

        std::getline(iss, cell, DELIMITER_CLEAN); // L
        remove_spaces_from_this(cell);
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: PGPhone column of row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << nine_digit_phone_number(cell) << DELIMITER_CLEAN; // PGPhone
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // M
        // ignore column

        std::getline(iss, cell, DELIMITER_CLEAN); // N
        // ignore column

        std::getline(iss, cell, DELIMITER_CLEAN); // O
        // ignore column

        std::getline(iss, cell, DELIMITER_CLEAN); // P
        // ignore column

        std::getline(iss, cell, DELIMITER_CLEAN); // Q
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: Race column of row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // Race
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // R
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: Birthdate column of row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // Birthdate
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // S
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: Gender column of row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // Gender
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // T
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: Weight column of row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // Weight
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // U
        remove_spaces_from_this(cell);
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: Height (ft) column of row " + std::to_string(row_number)) << std::endl;
            ; // do nothing if empty (explicit here to emphasize adding the delimiter for the next cell)
        }
        else // cell is not empty
        {
            output << cell + '-'; // setting up for Height
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // V
        remove_spaces_from_this(cell);
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: Height (in) column of row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // Height
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // W
        // ignore column due to inconsistency in csv to xlsx

        std::getline(iss, cell, DELIMITER_CLEAN); // X
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ WARNING ] Blank cell: Sport column of row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // Sport
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // Y
        // ignore column

        if (inEnglish) // form filled out in English
        {
            std::getline(iss, cell, DELIMITER_CLEAN); // Z
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: English OMI column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // OMI
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AA
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: English Meds column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // Meds
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AB
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: English ExPain column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // ExPain
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AC
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: English Sync column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // Sync
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AD
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: English SOB column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // SOB
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AE
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: English Murmur column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // Murmur
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AF
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: English HiBP column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // HiBP
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AG
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: English FamHist column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // FamHist
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AH
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: English OMI column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // SCD
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AI
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: English FamDisabled column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // FamDisabled
            }
        }
        else // form filled out in another language
        {
            for (int i = 0; i < NUM_LANG_FIELDS; i++)
            {
                std::getline(iss, cell, DELIMITER_CLEAN); // ignore English columns
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AJ
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: Spanish OMI column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // OMI
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AK
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: Spanish Meds column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // Meds
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AL
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: Spanish ExPain column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // ExPain
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AM
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: Spanish Sync column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // Sync
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AN
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: Spanish SOB column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // SOB
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AO
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: Spanish Murmur column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // Murmur
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AP
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: Spanish HiBP column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // HiBP
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AQ
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: Spanish FamHist column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // FamHist
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AR
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: Spanish SCD column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // SCD
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AS
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ WARNING ] Blank cell: Spanish FamDisabled column of row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // FamDisabled
            }
        }

        if (file_exists(NAME_FORM_PATH, consent_form_file_name(NAME_EVENT, MSN, LastName, FirstName, NAME_FORM))) // Consent
        {
            output << FORM_YES << '\n'; // consent form exists; last cell, move to next row
        }
        else
        {
            output << FORM_NO << '\n'; // consent form does not exist; last cell, move to next row
        }
    }

    output.close(); // finished with producing proper output for Cardea (still may contain duplicates)
}

void RemoveDuplicatesFrom(std::ifstream& input)
{
    /* Generates proper output file for Cardea with duplicates removed */
    std::ofstream output;
    output.open(NAME_OUTPUT);

    /* Third parser removes duplicates by iterating once to find duplicates then once again to remove them */
    for (auto header : HEADERS)
        output << header << DELIMITER_CSV; // add header row
    output << '\n'; // move to next row
    
    int current_iteration = 0;
    while (current_iteration < 2)
    {
        int row_number = 1; // headers was row 1
        std::string row; // holds current row of output with duplicates file
        input.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ignore header row
        while (std::getline(input, row)) // iterate through every row
        {
            row_number++; // increment current row number

            std::istringstream iss(row); // allows row to be broken down into cells separated by clean delimiter
            std::string cell; // holds contents of current cell of current row
            std::string MSN, LastName, FirstName; // will be used to check for filled consent forms

            std::getline(iss, cell, DELIMITER_CLEAN); // A (MSN)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // B (LastName)
            remove_duplicates_including_this(cell); // use this cell as a marker for a duplicate to be removed
            warn_duplicates_including_this(cell); // use this cell as a marker for a duplicate to be warned of

            std::getline(iss, cell, DELIMITER_CLEAN); // C (FirstName)
            remove_duplicates_including_this(cell); // use this cell as a marker for a duplicate to be removed
            warn_duplicates_including_this(cell); // use this cell as a marker for a duplicate to be warned of

            std::getline(iss, cell, DELIMITER_CLEAN); // D (Email)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // E (PGNam)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // F (PGPhone)
            remove_duplicates_including_this(cell); // use this cell as a marker for a duplicate to be removed

            std::getline(iss, cell, DELIMITER_CLEAN); // G (Race)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // H (Birthdate)
            remove_duplicates_including_this(cell); // use this cell as a marker for a duplicate to be removed

            std::getline(iss, cell, DELIMITER_CLEAN); // I (Gender)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // J (Weight)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // K (Height)
            // ignore column
            
            std::getline(iss, cell, DELIMITER_CLEAN); // L (Sport)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // M (OMI)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // N (Meds)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // O (ExPain)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // P (Sync)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // Q (SOB)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // R (Murmur)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // S (HiBP)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // T (FamHist)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // U (SCD)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // V (FamDisabled)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // W (Consent)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // X (Notes)
            // ignore column

            if (current_iteration == 0)
            {
                auto found = rows_scanned.find(potential_duplicate_to_warn);
                if (found == rows_scanned.end())
                {
                    rows_scanned.insert(potential_duplicate_to_warn);
                }
                else
                {
                    list_of_duplicates.insert(potential_duplicate_to_warn);
                }
                potential_duplicate_to_warn.erase();

                found = rows_scanned.find(potential_duplicate_to_remove);
                if (found == rows_scanned.end())
                {
                    rows_scanned.insert(potential_duplicate_to_remove);
                }
                else
                {
                    list_of_duplicates.insert(potential_duplicate_to_remove);
                }
                potential_duplicate_to_remove.erase();
            }
            else if (current_iteration == 1)
            {
                auto found = list_of_duplicates.find(potential_duplicate_to_remove);
                if (found == list_of_duplicates.end())
                {
                    std::replace(row.begin(), row.end(), DELIMITER_CLEAN, DELIMITER_CSV);
                    output << row << '\n';

                    found = list_of_duplicates.find(potential_duplicate_to_warn);
                    if (found != list_of_duplicates.end())
                    {
                        list_of_duplicates.erase(found, std::next(found));
                        std::cout << add_log("[ WARNING ] Potential duplicate: Row " + std::to_string(row_number)) << std::endl;
                    }
                    potential_duplicate_to_warn.erase();
                }
                else
                {
                    list_of_duplicates.erase(found, std::next(found));
                    std::cout << add_log("[ INFO ] Duplicate removed: Row " + std::to_string(row_number)) << std::endl;

                    found = list_of_duplicates.find(potential_duplicate_to_warn);
                    list_of_duplicates.erase(found, std::next(found));
                }
                potential_duplicate_to_remove.erase();
            }
        }

        input.clear();
        input.seekg(0, std::ios::beg);
        current_iteration++;
    }

    output.close(); // finished with producing proper output for Cardea with duplicates removed
}

void remove_spaces_from_this(std::string& entry)
{
    /*
        Adapted from:
        http://www.cplusplus.com/forum/beginner/191482/
    */
    const std::string copy = entry; // make a template of string to remove spaces from
    entry.clear(); // delete original string to rebuild from template without the spaces
    for (char c : copy) // iterate character by character through the template
    {
        if (!std::isspace(c))
        {
            entry += c; // only add a character to the original string if it is not a space
        }
    }
}

bool file_exists(std::string path, std::string file) {
    /*
        Adapted from:
        https://www.reddit.com/r/cpp_questions/comments/amswlp/detecting_if_a_directory_exists_in_visual_c/efod05p/
    */
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.path() == (path + '/' + file)) { // any slash works
            return true; // file from path exists
        }
    }
    return false; // file from path does not exist
}

std::string nine_digit_phone_number(std::string digits)
{
    return '(' + digits.substr(0, 3) + ')' + ' ' + digits.substr(3, 3) + '-' + digits.substr(6, 4);
}

std::string consent_form_file_name(std::string screeningName, std::string ID, std::string LN, std::string FN, std::string format)
{
    std::istringstream iss(format); // allows consent form file name formatting to be broken down into parts to insert variables
    std::vector<std::string> variable = { screeningName, ID, LN, FN }; // variables of consent form file name
    std::string part; // a part of the consent form file name
    std::string full; // full consent form file name with variables inserted
    int i = 0; // keeps track of which variable to insert

    iss >> part; // get first part from consent form file name (part before first whitespace)
    full += part; // add first part to the final consent form file name
    while (iss.good()) // iterate through all parts
    {
        iss >> part; // get next part from consent form file name (part before next whitespace)
        full += variable[i] + part; // add next part to the final consent form file name
        i++; // use next variable next time
    }

    return full; // produced full consent form file name with variables inserted
}

void warn_duplicates_including_this(std::string entry)
{
    potential_duplicate_to_warn += entry + DELIMITER_CLEAN;
}

void remove_duplicates_including_this(std::string entry)
{
    potential_duplicate_to_remove += entry + DELIMITER_CLEAN;
}

void CreateLogFile()
{
    std::cout << "Generating log file '" + NAME_LOG + "' . . .\n" << std::endl;
    
    std::ofstream logfile;
    logfile.open(NAME_LOG, std::ofstream::app); // if log already exists, append to it

    logfile << "=================================================================================" << std::endl;
    logfile << "NOTE: Any locations are referencing the raw input .csv file, not the output file!" << std::endl;
    logfile << "Events that occurred during the last run of the program:\n" << std::endl;
    for (auto log : logs)
    {
        logfile << log << std::endl;
    }

    logfile << "\n\nThe program was last run using the following parameters:\n" << std::endl;
    logfile <<
        "NAME_INPUT \t\t: \"" + NAME_INPUT + "\"\n"
        "NAME_OUTPUT \t\t: \"" + NAME_OUTPUT + "\"\n"
        "NAME_EVENT \t\t: \"" + NAME_EVENT + "\"\n"
        "NAME_FORM_PATH \t\t: \"" + NAME_FORM_PATH + "\"\n"
        "FORM_YES \t\t: \"" + FORM_YES + "\"\n"
        "FORM_NO \t\t: \"" + FORM_NO + "\"\n"
        "NAME_LOG \t\t: \"" + NAME_LOG + "\"\n"
        "NAME_INPUT_CLEAN \t: \"" + NAME_INPUT_CLEAN + "\"\n"
        "NAME_OUTPUT_DUPLICATES \t: \"" + NAME_OUTPUT_DUPLICATES + "\"\n"
        "NAME_ENGLISH \t\t: \"" + NAME_ENGLISH + "\"\n"
        "DELIMITER_CLEAN \t: '" + DELIMITER_CLEAN + "'\n"
        "DELIMITER_CSV \t\t: '" + DELIMITER_CSV + "'\n"
        << std::endl;

    logfile.close();
}

std::string add_log(std::string log)
{
    logs.push_back(log);

    return log; // return the original log string for convenience of printing messages to console
}

std::string init_NAME_INPUT()
{
    std::string name;
    std::cout << "What is the name of the input file? (.csv will be automaticaly appended) (NAME_INPUT)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty())
    {
        return "inputForCardea.csv";
    }
    else
    {
        return name + ".csv"; // automatically append .csv extension to name
    }
}

std::string init_NAME_OUTPUT()
{
    std::string name;
    std::cout << "How should the output file be named? (.csv will be automaticaly appended) (NAME_OUTPUT)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty())
    {
        return "outputForCardea.csv";
    }
    else
    {
        return name + ".csv"; // automatically append .csv extension to name
    }
}

std::string init_NAME_EVENT()
{
    std::string name;
    std::cout << "What is the screening name according to the consent form file name? (NAME_EVENT)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty())
    {
        return "screeningName";
    }
    else
    {
        return name;
    }
}

std::string init_NAME_FORM_PATH()
{
    std::string name;
    std::cout << "What is the path to the consent forms? (NAME_FORM_PATH)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty())
    {
        return "C:\\Users";
    }
    else
    {
        return name;
    }
}

std::string init_FORM_YES()
{
    std::string yes;
    std::cout << "What should be put in the Consent column if the consent form exists? (FORM_YES)" << std::endl;
    std::getline(std::cin, yes);
    if (yes.empty())
    {
        return "Yes";
    }
    else
    {
        return yes;
    }
}

std::string init_FORM_NO()
{
    std::string no;
    std::cout << "What should be put in the Consent column if the consent form does not exist? (FORM_NO)" << std::endl;
    std::getline(std::cin, no);
    if (no.empty())
    {
        return "";
    }
    else
    {
        return no;
    }
}

std::string init_NAME_LOG()
{
    std::string name;
    std::cout << "[ ADVANCED ] How should the log file detailing program events be named? (NAME_LOG)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty())
    {
        return "log_" + NAME_EVENT + ".txt";
    }
    else
    {
        return name; // extension not automatically appended
    }
}

std::string init_NAME_INPUT_CLEAN()
{
    std::string name;
    std::cout << "[ ADVANCED ] How should the sanitized input be named? (NAME_INPUT_CLEAN)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty())
    {
        return "sanitized.csv";
    }
    else
    {
        return name; // extension not automatically appended
    }
}

std::string init_NAME_OUTPUT_DUPLICATES()
{
    std::string name;
    std::cout << "[ ADVANCED ] How should the output still containing duplicates be named? (NAME_OUTPUT_DUPLICATES)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty())
    {
        return "duplicates.csv";
    }
    else
    {
        return name; // extension not automatically appended
    }
}

std::string init_NAME_ENGLISH()
{
    std::string name;
    std::cout << "[ ADVANCED ] How is it indicated in the raw input csv that the forms were filled out in English? (NAME_ENGLISH)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty())
    {
        return "English";
    }
    else
    {
        return name;
    }
}

char init_DELIMITER_CLEAN()
{
    std::string delimiter;
    std::cout << "[ ADVANCED ] What single character should be the clean delimiter for the sanitized input csv? (DELIMITER_CLEAN)" << std::endl;
    std::getline(std::cin, delimiter);
    if (delimiter.empty())
    {
        return '$';
    }
    else
    {
        return delimiter[0]; // returns only first character of input
    }
}

char init_DELIMITER_CSV()
{
    std::string delimiter;
    std::cout << "[ ADVANCED ] What is the delimiter of the raw input csv? (DELIMITER_CSV)" << std::endl;
    std::getline(std::cin, delimiter);
    if (delimiter.empty())
    {
        return ',';
    }
    else
    {
        return delimiter[0]; // returns only first character of input
    }
}