/* Ignore; needed to run program properly */
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include <iterator>
#include <filesystem> // requires C++17

/* Ignore; for parameter initialization upon program start */
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

/* Can freely initialize any of the following parameters */
const std::string NAME_INPUT = init_NAME_INPUT(); // name of input csv to parse
const std::string NAME_OUTPUT = init_NAME_OUTPUT(); // desired name of parsed output csv for Cardea
const std::string NAME_EVENT = init_NAME_EVENT(); // name of current screening event based on naming of consent form file
const std::string NAME_FORM_PATH = init_NAME_FORM_PATH(); // directory path to the consent forms; any slash works
const std::string FORM_YES = init_FORM_YES(); // what to print into Consent column of output csv if consent form exists
const std::string FORM_NO = init_FORM_NO(); // what to print into Consent column of output csv if consent form does not exist

/* Should be more wary about initializing the following parameters */
const std::string NAME_LOG = init_NAME_LOG(); // desired name of log file detailing program events
const std::string NAME_INPUT_CLEAN = init_NAME_INPUT_CLEAN(); // ignore; needed to sanitize input properly
const std::string NAME_OUTPUT_DUPLICATES = init_NAME_OUTPUT_DUPLICATES(); // ignore; needed to remove duplicates properly
const std::string NAME_ENGLISH = init_NAME_ENGLISH(); // how it was indicated in the raw input csv that the forms were filled out in English
const char DELIMITER_CLEAN = init_DELIMITER_CLEAN(); // which singular character to forbid while filling out forms; needed to sanitize input properly
const char DELIMITER_CSV = init_DELIMITER_CSV(); // how raw input csv delimits cells; needs to be cleaned out by sanitizer function

/* Cannot initialize the following parameters since source code might have to be adjusted */
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
void Parse(); // runs the program workflow: Sanitize() -> MakeCardeaCompatible() -> RemoveDuplicatesFrom()
void Sanitize(std::ifstream& input); // sanitizes input and produces sanitized output file for processing
void MakeCardeaCompatible(std::ifstream& input); // parses sanitized input according to Cardea, produces proper output for Cardea
void RemoveDuplicatesFrom(std::ifstream& input); // produces output for Cardea without duplicates and/or with warnings of them
void CreateLogFile(); // appends to log file significant program events

/* Assets intended for MakeCardeaCompatible() */
void remove_spaces_from_this(std::string& entry); // any entry given will have its whitespaces removed
bool file_exists(std::string path, std::string file); // tells whether file from given path exists
std::string nine_digit_phone_number(std::string digits); // formats raw nine digits into a phone number compatible with Cardea
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

/* Actual program */
int main()
{
    std::cout << "\nEvents that occurred during the last run of the program:\n" << std::endl;

    Parse(); // run the parser functions based on configured parameters

    CreateLogFile(); // appends to log file the significant events that occurred during the program

    std::cout << "You may close the program or press ENTER to exit." << std::endl;
    std::cin.ignore();

    return 0; // end of program
}

/* Function definitions */
/* Parser functions used in the program */
void Parse()
{
   ///////////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////// PART 1: SANITIZE RAW INPUT CSV ////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////

   /* Ignore; checks if raw input csv can be opened */
    std::ifstream raw_input(NAME_INPUT);
    if (!raw_input)
    {
        std::cout << add_log("[ ERROR ] '" + NAME_INPUT + "' could not be opened or "
            + "does not exist.")
            << "\nMake sure the input .csv is in the same folder as this program." << std::endl;
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
        std::cout << add_log("[ ERROR ] Could not open sanitized input file.") << std::endl;
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
        std::cout << add_log("[ ERROR ] Could not open output with duplicates file.") << std::endl;
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
            std::cout << add_log("[ BLANK ] Blank cell found in MSN column of Row " + std::to_string(row_number)) << std::endl;
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
            std::cout << add_log("[ BLANK ] Blank cell found in Language column of Row " + std::to_string(row_number)) << std::endl;
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
            std::cout << add_log("[ BLANK ] Blank cell found in LastName column of Row " + std::to_string(row_number)) << std::endl;
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
            std::cout << add_log("[ BLANK ] Blank cell found in FirstName column of Row " + std::to_string(row_number)) << std::endl;
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
            std::cout << add_log("[ BLANK ] Blank cell found in Email column of Row " + std::to_string(row_number)) << std::endl;
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
            std::cout << add_log("[ BLANK ] Blank cell found in PGNam column of Row " + std::to_string(row_number)) << std::endl;
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
            std::cout << add_log("[ BLANK ] Blank cell found in PGPhone column of Row " + std::to_string(row_number)) << std::endl;
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
            std::cout << add_log("[ BLANK ] Blank cell found in Race column of Row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // Race
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // R
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ BLANK ] Blank cell found in Birthdate column of Row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // Birthdate
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // S
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ BLANK ] Blank cell found in Gender column of Row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            output << cell << DELIMITER_CLEAN; // Gender
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // T
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ BLANK ] Blank cell found in Weight column of Row " + std::to_string(row_number)) << std::endl;
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
            std::cout << add_log("[ BLANK ] Blank cell found in Height (ft) column of Row " + std::to_string(row_number)) << std::endl;
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
            std::cout << add_log("[ BLANK ] Blank cell found in Height (in) column of Row " + std::to_string(row_number)) << std::endl;
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
            std::cout << add_log("[ BLANK ] Blank cell found in Sport column of Row " + std::to_string(row_number)) << std::endl;
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
                std::cout << add_log("[ BLANK ] Blank cell found in English OMI column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // OMI
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AA
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in English Meds column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // Meds
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AB
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in English ExPain column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // ExPain
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AC
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in English Sync column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // Sync
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AD
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in English SOB column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // SOB
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AE
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in English Murmur column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // Murmur
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AF
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in English HiBP column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // HiBP
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AG
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in English FamHist column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // FamHist
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AH
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in English OMI column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // SCD
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AI
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in English FamDisabled column of Row " + std::to_string(row_number)) << std::endl;
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
                std::cout << add_log("[ BLANK ] Blank cell found in Spanish OMI column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // OMI
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AK
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in Spanish Meds column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // Meds
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AL
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in Spanish ExPain column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // ExPain
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AM
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in Spanish Sync column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // Sync
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AN
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in Spanish SOB column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // SOB
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AO
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in Spanish Murmur column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // Murmur
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AP
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in Spanish HiBP column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // HiBP
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AQ
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in Spanish FamHist column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // FamHist
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AR
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in Spanish SCD column of Row " + std::to_string(row_number)) << std::endl;
                output << DELIMITER_CLEAN; // leave cell blank in output if empty
            }
            else // cell is not empty
            {
                output << cell << DELIMITER_CLEAN; // SCD
            }

            std::getline(iss, cell, DELIMITER_CLEAN); // AS
            if (cell.empty()) // checks if cell is empty
            {
                std::cout << add_log("[ BLANK ] Blank cell found in Spanish FamDisabled column of Row " + std::to_string(row_number)) << std::endl;
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

    /* Third parser removes duplicates by iterating once to find duplicates then once again to take action against them */
    for (auto header : HEADERS)
        output << header << DELIMITER_CSV; // add header row
    output << '\n'; // move to next row
    
    int current_iteration = 0; // tracks current iteration
    while (current_iteration < 2) // will iterate through input twice
    {
        int row_number = 1; // headers was row 1
        std::string row; // holds current row of output with duplicates file
        input.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ignore header row
        while (std::getline(input, row)) // iterate through every row
        {
            row_number++; // increment current row number

            std::istringstream iss(row); // allows row to be broken down into cells separated by clean delimiter
            std::string cell; // holds contents of current cell of current row

            std::getline(iss, cell, DELIMITER_CLEAN); // A (MSN)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // B (LastName)
            track_duplicates_including_this(cell, true, true, true); // track this column to warn, for swap, and to remove duplicates

            std::getline(iss, cell, DELIMITER_CLEAN); // C (FirstName)
            track_duplicates_including_this(cell, true, true, true); // track this column to warn, for swap, and to remove duplicates

            std::getline(iss, cell, DELIMITER_CLEAN); // D (Email)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // E (PGNam)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // F (PGPhone)
            track_duplicates_including_this(cell, false, false, true); // track this column to remove duplicates

            std::getline(iss, cell, DELIMITER_CLEAN); // G (Race)
            // ignore column

            std::getline(iss, cell, DELIMITER_CLEAN); // H (Birthdate)
            track_duplicates_including_this(cell, false, false, true); // track this column to remove duplicates

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

            if (current_iteration == 0) // first iteration through input; purpose is to find the duplicates
            {
                if (!potential_duplicate_to_remove.empty()) // only track duplicates to remove if a column was specified to
                {
                    auto find_to_remove = rows_scanned_for_duplicates.find(potential_duplicate_to_remove); // scan string list for current row string
                    if (find_to_remove == rows_scanned_for_duplicates.end()) // scan reached end of string list (did not find a match)
                    {
                        rows_scanned_for_duplicates.insert(potential_duplicate_to_remove); // add current row string to string list
                    }
                    else // scan did not reach end of string list (a match was found)
                    {
                        list_of_duplicates.insert(potential_duplicate_to_remove); // add current row string to list of confirmed duplicates
                        list_of_duplicates_found.emplace(potential_duplicate_to_remove, std::vector<int>()); // start a record of involved row locations
                    }
                }

                if (!potential_duplicate_to_warn.empty()) // only track duplicates to warn if a column was specified to
                {
                    auto find_to_warn = rows_scanned_for_duplicates.find(potential_duplicate_to_warn); // scan string list for current row string
                    if (find_to_warn == rows_scanned_for_duplicates.end()) // scan reached end of string list (did not find a match)
                    {
                        rows_scanned_for_duplicates.insert(potential_duplicate_to_warn); // add current row string to string list
                    }
                    else // scan did not reach end of string list (a match was found)
                    {
                        list_of_duplicates.insert(potential_duplicate_to_warn); // add current row string to list of confirmed duplicates
                        list_of_duplicates_found.emplace(potential_duplicate_to_warn, std::vector<int>()); // start a record of involved row locations
                    }
                }

                if (!potential_swap.empty()) // only track for swap if a column was specified to
                {
                    auto find_swap = rows_scanned_for_swaps.find(potential_swap); // scan string list for current row string
                    if (find_swap == rows_scanned_for_swaps.end()) // scan reached end of string list (did not find a match)
                    {
                        rows_scanned_for_swaps.insert(swapped(potential_swap)); // add swapped version of current row string to string list
                    }
                    else // scan did not reach end of string list (a match was found)
                    {
                        list_of_swaps.insert(potential_swap); // add current row string to list of confirmed swaps
                        list_of_swaps_found.emplace(swapped(potential_swap), std::vector<int>()); // start a record of involved row locations; base on original
                    }
                }
            }
            else if (current_iteration == 1) // second iteration through input; purpose is to take action against the found duplicates
            {
                /* Take action against duplicates */
                auto find_to_remove = list_of_duplicates.find(potential_duplicate_to_remove); // scan string list of confirmed duplicates for row to remove
                if (find_to_remove == list_of_duplicates.end()) // scan reached end of string list (did not find a row to remove)
                {
                    /* Build the final output file; includes every row from input (even warnings and swaps) unless it is a duplicate to remove */
                    std::replace(row.begin(), row.end(), DELIMITER_CLEAN, DELIMITER_CSV); // replace the clean delimiter with original delimiter for this row
                    output << row << '\n'; // add this row to the final output (this row is not a duplicate to remove)

                    auto find_to_remove_last = list_of_duplicates_found.find(potential_duplicate_to_remove); // see if this row is the reference duplicate
                    if (find_to_remove_last != list_of_duplicates_found.end()) // scan did not reach end (a match was found)
                    {
                        find_to_remove_last->second.push_back(row_number); // record row number; last number will always be reference duplicate (the latest row)
                    }

                    auto find_to_warn = list_of_duplicates.find(potential_duplicate_to_warn); // scan string list of confirmed duplicates for row to warn of
                    if (find_to_warn == list_of_duplicates.end()) // scan reached end of string list (did not find a row to warn of)
                    {
                        auto find_to_warn_last = list_of_duplicates_found.find(potential_duplicate_to_warn); // see if this row is the reference duplicate
                        if (find_to_warn_last != list_of_duplicates_found.end()) // scan did not reach end (a match was found)
                        {
                            find_to_warn_last->second.push_back(row_number); // record row number; last number will always be reference duplicate (the latest row)
                        }
                    }
                    else // scan did not reach end of string list (a row to warn of duplicate was found)
                    {
                        list_of_duplicates.erase(find_to_warn, std::next(find_to_warn)); // remove from confirmed list just one instance of duplicate to warn of
                        auto record_row = list_of_duplicates_found.find(potential_duplicate_to_warn); // get the record of involved row locations for this row
                        record_row->second.push_back(row_number); // add current row number to the record
                    }
                }
                else // scan did not reach end of string list (a row to remove was found)
                {
                    // row removed by not adding it to the final outfile

                    list_of_duplicates.erase(find_to_remove, std::next(find_to_remove)); // remove from confirmed list just one instance of duplicate to remove
                    auto record_row = list_of_duplicates_found.find(potential_duplicate_to_remove); // get the record of involved row locations for this row
                    record_row->second.push_back(row_number); // add current row number to the record

                    /* Do not add to the record a warning of row duplicate if that same row was already decided to be removed */
                    auto find_to_warn = list_of_duplicates.find(potential_duplicate_to_warn);
                    if (find_to_warn != list_of_duplicates.end())
                    {
                        list_of_duplicates.erase(find_to_warn, std::next(find_to_warn)); // still remove from confirmed list the duplicate to warn
                    }
                }

                /* Take action against swaps */
                auto find_swapped_from = list_of_swaps_found.find(potential_swap); // see if this row is the original (reference swap)
                if (find_swapped_from != list_of_swaps_found.end()) // scan did not reach end (a match was found)
                {
                    std::vector<int>& record_of_row_numbers = find_swapped_from->second; // give a nice name to reference the record of row locations
                    if (record_of_row_numbers.empty()) // no row locations recorded yet for this swap
                    {
                        record_of_row_numbers.push_back(row_number); // first number added will be the row number of the reference swap
                    }
                    else
                    {
                        record_of_row_numbers[0] = row_number; // ensure first number will always be the latest row number of the reference swap
                    }
                }
                auto find_swap = list_of_swaps.find(potential_swap); // scan string list of confirmed swaps for row string
                if (find_swap != list_of_swaps.end()) // scan did not reach end of string list (a swap was found)
                {
                    list_of_swaps.erase(find_swap, std::next(find_swap)); // remove from confirmed list just one instance of swap
                    auto record_row = list_of_swaps_found.find(swapped(potential_swap)); // get the record of involved row locations for this row
                    record_row->second.push_back(row_number); // add current row number to the record
                }
            }

            /* Required for proper function; clears the strings of current row for the next row */
            potential_duplicate_to_remove.erase();
            potential_duplicate_to_warn.erase();
            potential_swap.erase();
        }

        /* Required for proper function; sets up for next iteration of input */
        input.clear(); // clears flags that end of document was reached
        input.seekg(0, std::ios::beg); // start scanning from beginning of document
        current_iteration++; // current iteration complete, next one begins
    }

    output.close(); // finished with producing proper output for Cardea with duplicates removed

    /* Add events to the log */
    for (auto found : list_of_duplicates_found) // go through every unique duplicate found
    {
        for (auto row_num : found.second) // access the records of row locations of the duplicates
        {
            if (row_num == found.second.back()) break; // done logging once row of reference duplicate is reached (the latest row)

            /*  Recall that potential duplicates to remove are tagged by the delimiter in the front! See track_duplicates_including_this() */
            if (found.first[0] == DELIMITER_CLEAN) // is a potential duplicate to remove
            {
                std::cout << add_log("[ DELETION ] Removed duplicate of Row " + std::to_string(found.second.back())
                    + " that was found earlier on Row " + std::to_string(row_num)) << std::endl;
            }
            else // is a potential duplicate to warn of
            {
                std::cout << add_log("[ WARNING ] Potential duplicate of Row " + std::to_string(found.second.back())
                    + " was found earlier on Row " + std::to_string(row_num)) << std::endl;
            }
        }
    }
    for (auto found : list_of_swaps_found) // go through every unique swap found
    {
        for (auto row_num : found.second) // access the records of row locations of the swaps
        {
            if (row_num == found.second.front()) continue; // skip logging first number since it is the row number of the reference swap

            std::cout << add_log("[ WARNING ] Potential duplicate (swapped first/last names) between Row " + std::to_string(row_num)
                + " and Row " + std::to_string(found.second.front())) << std::endl;
        }
    }
}

void CreateLogFile()
{
    std::cout << "\nCopying to log file '" + NAME_LOG + "' . . .\n" << std::endl;

    std::ofstream logfile;
    logfile.open(NAME_LOG, std::ofstream::app); // if log file already exists, append to it (not write over it)

    logfile << "=================================================================================" << std::endl;
    logfile << "NOTE: Any locations are referencing the raw input .csv file, not the output file!" << std::endl;
    logfile << "Events that occurred during the last run of the program:\n" << std::endl;
    for (auto log : logs)
    {
        logfile << log << std::endl; // write every log to log file
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

/* Functions intended for MakeCardeaCompatible() */
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

bool file_exists(std::string path, std::string file)
{
    /*
        Adapted from:
        https://www.reddit.com/r/cpp_questions/comments/amswlp/detecting_if_a_directory_exists_in_visual_c/efod05p/
    */
    if (std::filesystem::exists(path)) // default path might not be valid; check to ensure path is valid
    {
        for (const auto& entry : std::filesystem::directory_iterator(path)) // assumes path is valid; otherwise, program crashes
        {
            if (entry.path() == (path + '/' + file)) // any slash works
            {
                return true; // file from path exists
            }
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

/* Functions intended for RemoveDuplicatesFrom() */
void track_duplicates_including_this(std::string entry, bool to_warn, bool for_swap, bool to_remove)
{
    std::transform(entry.begin(), entry.end(), entry.begin(), ::tolower);

    if (to_warn)
    {
        potential_duplicate_to_warn += entry + DELIMITER_CLEAN;
    }

    if (for_swap)
    {
        potential_swap += entry + DELIMITER_CLEAN;
    }

    if (to_remove)
    {
        potential_duplicate_to_remove += DELIMITER_CLEAN + entry; // all potential duplicates to remove are tagged by the delimiter in the front!
    }
}

std::string swapped(std::string potential_duplicate)
{
    std::istringstream iss(potential_duplicate);
    std::string cell;
    std::getline(iss, cell, DELIMITER_CLEAN);
    std::string first_cell = cell;
    std::getline(iss, cell, DELIMITER_CLEAN);
    std::string second_cell = cell;
    std::getline(iss, cell);
    std::string other_cells = cell;
    
    std::string original_copy = potential_swap;
    potential_swap.erase();
    track_duplicates_including_this(second_cell, false, true, false);
    track_duplicates_including_this(first_cell, false, true, false);
    std::string swapped_copy = potential_swap + other_cells;
    potential_swap = original_copy;
    return swapped_copy;
}

/* Functions for CreateLogFile() */
std::string add_log(std::string log)
{
    logs.push_back(log);

    return log; // return the original log string for convenience of printing messages to console
}

/* Functions for parameter initialization upon program start */
std::string init_NAME_INPUT()
{
    const std::string default_name = "inputForCardea";

    std::string name;
    std::cout << "What is the name of the input file? (.csv will be automaticaly appended) (NAME_INPUT)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty()) // nothing entered, use default
    {
        std::cout << "Nothing was entered. Default name will be used (ignore quotes): \"" << default_name << "\"" << std::endl;
        return default_name + ".csv"; // automatically append .csv extension to name
    }
    else
    {
        return name + ".csv"; // automatically append .csv extension to name
    }
}

std::string init_NAME_OUTPUT()
{
    const std::string default_name = "outputForCardea";

    std::string name;
    std::cout << "How should the output file be named? (.csv will be automaticaly appended) (NAME_OUTPUT)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty()) // nothing entered, use default
    {
        std::cout << "Nothing was entered. Default name will be used (ignore quotes): \"" << default_name << "\"" << std::endl;
        return default_name + ".csv"; // automatically append .csv extension to name
    }
    else
    {
        return name + ".csv"; // automatically append .csv extension to name
    }
}

std::string init_NAME_EVENT()
{
    const std::string default_name = "screeningName";
    
    std::string name;
    std::cout << "What is the screening name according to the consent form file name? (NAME_EVENT)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty()) // nothing entered, use default
    {
        std::cout << "Nothing was entered. Default name will be used (ignore quotes): \"" << default_name << "\"" << std::endl;
        return default_name;
    }
    else
    {
        return name;
    }
}

std::string init_NAME_FORM_PATH()
{
    const std::string default_name = "C:\\Users\\Bryan\\SHF\\Heart Screenings\\Forms"; // escaped characters; C:\Users\Bryan\SHF\"Heart Screenings"\Forms
    
    std::cout << "What is the path to the consent forms? (NAME_FORM_PATH)" << std::endl;
    while (true) // keep prompting for path name until a valid path is given
    {
        std::string name;
        std::getline(std::cin, name);
        if (name.empty()) // nothing entered, use default
        {
            std::cout << "Nothing was entered. Default path will be used (ignore quotes): \"" << default_name << "\"" << std::endl;
            return default_name;
        }
        else
        {
            if (std::filesystem::exists(name)) // path is valid
            {
                return name;
            }
            else // path is not valid
            {
                std::cout << "Path name is invalid. Check for typos or if path exists. Slash direction does not matter." << std::endl;
            }
        }
    }
}

std::string init_FORM_YES()
{
    const std::string default_yes = "Yes";

    std::string yes;
    std::cout << "What should be put in the Consent column if the consent form exists? (FORM_YES)" << std::endl;
    std::getline(std::cin, yes);
    if (yes.empty()) // nothing entered, use default
    {
        std::cout << "Nothing was entered. Default entry will be used (ignore quotes): \"" << default_yes << "\"" << std::endl;
        return default_yes;
    }
    else
    {
        return yes;
    }
}

std::string init_FORM_NO()
{
    const std::string default_no = "";
    
    std::string no;
    std::cout << "What should be put in the Consent column if the consent form does not exist? (FORM_NO)" << std::endl;
    std::getline(std::cin, no);
    if (no.empty()) // nothing entered, use default
    {
        std::cout << "Nothing was entered. Default entry will be used (ignore quotes): \"" << default_no << "\"" << std::endl;
        return default_no;
    }
    else
    {
        return no;
    }
}

std::string init_NAME_LOG()
{
    const std::string default_name = "log_" + NAME_EVENT + ".txt";
    
    std::string name;
    std::cout << "[ ADVANCED ] How should the log file detailing program events be named? (NAME_LOG)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty()) // nothing entered, use default
    {
        std::cout << "Nothing was entered. Default name will be used (ignore quotes): \"" << default_name << "\"" << std::endl;
        return default_name;
    }
    else
    {
        return name; // extension not automatically appended
    }
}

std::string init_NAME_INPUT_CLEAN()
{
    const std::string default_name = "sanitized.csv";

    std::string name;
    std::cout << "[ ADVANCED ] How should the sanitized input be named? (NAME_INPUT_CLEAN)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty()) // nothing entered, use default
    {
        std::cout << "Nothing was entered. Default name will be used (ignore quotes): \"" << default_name << "\"" << std::endl;
        return default_name;
    }
    else
    {
        return name; // extension not automatically appended
    }
}

std::string init_NAME_OUTPUT_DUPLICATES()
{
    const std::string default_name = "duplicates.csv";
    
    std::string name;
    std::cout << "[ ADVANCED ] How should the output still containing duplicates be named? (NAME_OUTPUT_DUPLICATES)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty()) // nothing entered, use default
    {
        std::cout << "Nothing was entered. Default name will be used (ignore quotes): \"" << default_name << "\"" << std::endl;
        return default_name;
    }
    else
    {
        return name; // extension not automatically appended
    }
}

std::string init_NAME_ENGLISH()
{
    const std::string default_name = "English";
    
    std::string name;
    std::cout << "[ ADVANCED ] How is it indicated in the raw input csv that the forms were filled out in English? (NAME_ENGLISH)" << std::endl;
    std::getline(std::cin, name);
    if (name.empty()) // nothing entered, use default
    {
        std::cout << "Nothing was entered. Default name will be used (ignore quotes): \"" << default_name << "\"" << std::endl;
        return default_name;
    }
    else
    {
        return name;
    }
}

char init_DELIMITER_CLEAN()
{
    const char default_delimiter = '$';
    
    std::string delimiter;
    std::cout << "[ ADVANCED ] What single character should be the clean delimiter for the sanitized input csv? (DELIMITER_CLEAN)" << std::endl;
    std::getline(std::cin, delimiter);
    if (delimiter.empty()) // nothing entered, use default
    {
        std::cout << "Nothing was entered. Default delimiter will be used (ignore quotes): '" << default_delimiter << "'" << std::endl;
        return default_delimiter;
    }
    else
    {
        return delimiter[0]; // returns only first character of input
    }
}

char init_DELIMITER_CSV()
{
    const char default_delimiter = ',';
    
    std::string delimiter;
    std::cout << "[ ADVANCED ] What is the delimiter of the raw input csv? (DELIMITER_CSV)" << std::endl;
    std::getline(std::cin, delimiter);
    if (delimiter.empty()) // nothing entered, use default
    {
        std::cout << "Nothing was entered. Default delimiter will be used (ignore quotes): '" << default_delimiter << "'" << std::endl;
        return default_delimiter;
    }
    else
    {
        return delimiter[0]; // returns only first character of input
    }
}