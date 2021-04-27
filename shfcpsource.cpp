/* Ignore; needed to run program properly */
#include <iostream> // std::cout, std::cin, ::ignore()
#include <string> // std::string, std::string::c_str(), std::to_string(), std::stoi()
#include <vector> // std::vector
#include <unordered_set> // std::unordered_set, std::unordered_multiset
#include <unordered_map> // std::unordered_map
#include <fstream> // std::ifstream, std::ofstream, ::ignore()
#include <sstream> // std::istringstream
#include <cstdio> // std::remove()
#include <cctype> // std::isspace(), ::tolower, ::toupper
#include <algorithm> // std::replace(), std::transform()
#include <iterator> // std::next(), ::begin(), ::end()
#include <limits> // std::numeric_limits::max()
#include <filesystem> // std::filesystem::exists(), std::filesystem::directory_iterator() // requires C++17

/* Ignore; for welcome message and advanced setup */
std::string init_MODE();
const std::string MODE = init_MODE();

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
const std::string NAME_INPUT_CLEAN = init_NAME_INPUT_CLEAN(); // intermediate file needed to sanitize input properly
const std::string NAME_OUTPUT_DUPLICATES = init_NAME_OUTPUT_DUPLICATES(); // intermediate file needed to remove duplicates properly
const std::string NAME_ENGLISH = init_NAME_ENGLISH(); // how it was indicated in the raw input csv that the forms were filled out in English
const char DELIMITER_CLEAN = init_DELIMITER_CLEAN(); // which singular character to forbid while filling out forms; needed to sanitize input properly
const char DELIMITER_CSV = init_DELIMITER_CSV(); // how raw input csv delimits cells; needs to be cleaned out by sanitizer function

/* Cannot initialize the following parameters since source code might have to be adjusted */
const char NAME_FORM_VAR = '#'; // singular character to represent as the variables in NAME_FORM
const std::string NAME_FORM = "SHF-Consent_#_#_#_#-SIGNED.pdf"; // consent form file name, adding NAME_FORM_VAR where a variable is
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
void remove_spaces_from_this(std::string& entry); // any entry given will have its whitespace characters removed
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
   //////////////////////////////////// STAGE 1: SANITATION //////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////////////////////

   /* Checks if raw input csv can be opened */
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
    //////////////////////////// STAGE 2: CARDEA-COMPATIBLE CONVERSION ////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /* Checks if sanitized input file can be opened */
    std::ifstream sanitized_input(NAME_INPUT_CLEAN);
    if (!sanitized_input)
    {
        std::cout << add_log("[ ERROR ] Could not open sanitized input file.") << std::endl;
        return; // terminate program early due to error
    }

    /* Parse sanitized input file that was just produced */
    MakeCardeaCompatible(sanitized_input);

    sanitized_input.close(); // finished with parsing sanitized input file

    if (MODE == "RETAIN") // see Advanced Setup section in manual
    {
        std::cout << add_log("[ INFO ] Retained \"" + NAME_INPUT_CLEAN
            + "\" intermediate file.") << std::endl;
    }
    else
    {
        std::remove(NAME_INPUT_CLEAN.c_str()); // delete the sanitized input file
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////// STAGE 3: DUPLICATE ACTION ///////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /* Checks if output still with duplicates file can be opened */
    std::ifstream output_with_duplicates(NAME_OUTPUT_DUPLICATES);
    if (!output_with_duplicates)
    {
        std::cout << add_log("[ ERROR ] Could not open output with duplicates file.") << std::endl;
        return; // terminate program early due to error
    }

    /* Remove duplicates from output with duplicates file that was just produced */
    RemoveDuplicatesFrom(output_with_duplicates);

    output_with_duplicates.close(); // finished with removing duplicates from file

    if (MODE == "RETAIN") // see Advanced Setup section in manual
    {
        std::cout << add_log("[ INFO ] Retained \"" + NAME_OUTPUT_DUPLICATES
            + "\" intermediate file.") << std::endl;
    }
    else
    {
        std::remove(NAME_OUTPUT_DUPLICATES.c_str()); // delete the output with duplicates file
    }
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
        std::istringstream iss(row); // allows row to be broken down into cells separated by dirty delimiter
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
            if (cell.size() == 10 && (cell[0] == '0' || cell[0] == '1')) // USA phone number area codes cannot begin with 0 or 1
            {
                std::cout << add_log("[ WARNING ] Potentially invalid phone number found in PGPhone column of Row " 
                    + std::to_string(row_number)) << std::endl;
            }
            output << ten_digit_phone_number(cell) << DELIMITER_CLEAN; // PGPhone
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

        std::getline(iss, cell, DELIMITER_CLEAN); // U (Height (ft))
        remove_spaces_from_this(cell);
        int height_in_inches = -1; // initialized as negative number since height in inches can be zero or greater
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ BLANK ] Blank cell found in Height (ft) column of Row " + std::to_string(row_number)) << std::endl;
            ; // do nothing if empty (explicit here to emphasize adding the delimiter only in the next cell)
        }
        else // cell is not empty
        {
            height_in_inches = std::stoi(cell) * 12; // converts cell from string to integer for program to function properly
            output << cell + '-'; // setting up for Height column in output
        }

        std::getline(iss, cell, DELIMITER_CLEAN); // V (Height (in))
        remove_spaces_from_this(cell);
        if (cell.empty()) // checks if cell is empty
        {
            std::cout << add_log("[ BLANK ] Blank cell found in Height (in) column of Row " + std::to_string(row_number)) << std::endl;
            output << DELIMITER_CLEAN; // leave cell blank in output if empty
        }
        else // cell is not empty
        {
            if (std::stoi(cell) == height_in_inches) // ensures height (in) entered was not height (ft) converted to inches
            {
                cell = "0"; // correct the height (in); assumes such a mistake was made when actual height was exactly height (ft)
                std::cout << add_log("[ CORRECTION ] Height in inches was adjusted from " + std::to_string(height_in_inches)
                    + " inches to " + cell + " inches in Height (in) column of Row " + std::to_string(row_number)) << std::endl;
            }
            else if (std::stoi(cell) > 12) // warn if height (in) is greater than 12 inches but not equal to height (ft) in inches
            {
                std::cout << add_log("[ WARNING ] Height in inches was greater than 12 inches in Height (in) column of Row "
                    + std::to_string(row_number)) << std::endl;
            }
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

                    auto find_reference_duplicate_of_remove = list_of_duplicates_found.find(potential_duplicate_to_remove); // see if this row is the reference duplicate
                    if (find_reference_duplicate_of_remove != list_of_duplicates_found.end()) // scan did not reach end (a match was found)
                    {
                        /* Last number of row record will always be the reference duplicate */
                        find_reference_duplicate_of_remove->second.push_back(row_number); // record row number of reference duplicate
                    }

                    auto find_to_warn = list_of_duplicates.find(potential_duplicate_to_warn); // scan string list of confirmed duplicates for row to warn of
                    if (find_to_warn == list_of_duplicates.end()) // scan reached end of string list (did not find a row to warn of)
                    {
                        auto find_reference_duplicate_of_warn = list_of_duplicates_found.find(potential_duplicate_to_warn); // see if this row is the reference duplicate
                        if (find_reference_duplicate_of_warn != list_of_duplicates_found.end()) // scan did not reach end (a match was found)
                        {
                            /* Last number of row record will always be the reference duplicate */
                            find_reference_duplicate_of_warn->second.push_back(row_number); // record row number of reference duplicate
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
                    // row removed by not adding it to the final output file

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
                auto find_reference_swap = list_of_swaps_found.find(potential_swap); // see if this row is the original (reference swap)
                if (find_reference_swap != list_of_swaps_found.end()) // scan did not reach end (a match was found)
                {
                    /* First number of row record will always be the reference swap */
                    std::vector<int>& record_of_row_numbers = find_reference_swap->second; // give a nice name to reference the record of row locations
                    if (record_of_row_numbers.empty()) // no row locations of swaps recorded yet for this reference swap
                    {
                        record_of_row_numbers.push_back(row_number); // first number added is the row number of the reference swap
                    }
                    else // there is already at least a record of the row number of an older reference swap
                    {
                        record_of_row_numbers[0] = row_number; // ensure first number will always be the reference swap; replace older reference swap location
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
        for (auto row_num : found.second) // .second accesses the records of row locations of the duplicates
        {
            /* .second.back() is last number (.back()) in row record (.second) */
            if (row_num == found.second.back()) break; // skip logging last number since it is the row number of the reference duplicate

            /*  Recall that potential duplicates to remove are tagged by the delimiter in the front! See track_duplicates_including_this() */
            if (found.first[0] == DELIMITER_CLEAN) // .first[0] checks the first character of the row string; is a potential duplicate to remove if delimiter
            {
                std::cout << add_log("[ DELETION ] Removed duplicate of Row " + std::to_string(found.second.back())
                    + " that was found earlier on Row " + std::to_string(row_num)) << std::endl;
            }
            else // is a potential duplicate to warn of since first character of row string was not delimiter
            {
                std::cout << add_log("[ WARNING ] Potential duplicate of Row " + std::to_string(found.second.back())
                    + " was found earlier on Row " + std::to_string(row_num)) << std::endl;
            }
        }
    }
    for (auto found : list_of_swaps_found) // go through every unique swap found
    {
        for (auto row_num : found.second) // .second accesses the records of row locations of the swaps
        {
            /* .second.front() is first number (.front()) in row record (.second) */
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

    logfile << "\n\nThe program was last run using this mode: " + MODE << std::endl;

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

std::string ten_digit_phone_number(std::string digits)
{
    return '(' + digits.substr(0, 3) + ')' + ' ' + digits.substr(3, 3) + '-' + digits.substr(6, 4);
}

std::string consent_form_file_name(std::string screeningName, std::string ID, std::string LN, std::string FN, std::string format)
{
    std::istringstream iss(format); // allows consent form file name formatting to be broken down into parts to insert variables
    std::vector<std::string> variable = { screeningName, ID, LN, FN }; // variables of consent form file name in insertion order
    std::string format_part; // a part of the consent form file name
    std::string name_segment; // temporary segment when combining format_part and a variable
    std::string full_name; // full consent form file name with variables inserted
    const char var_symbol = NAME_FORM_VAR; // singular character used to signify where a variable is

    for (int i = 0; i < variable.size(); i++) // iterate through every variable to be present in the name
    {
        std::getline(iss, format_part, var_symbol); // scan through name template, stopping at the next variable symbol
        name_segment = format_part + variable[i]; // combine the part just scanned with a variable
        full_name += name_segment; // add on the segment to the final name
    }
    std::getline(iss, format_part); // retrieve the rest of the name template
    full_name += format_part; // add on the rest to the final name

    return full_name; // produced full consent form file name with variables inserted
}

/* Functions intended for RemoveDuplicatesFrom() */
void track_duplicates_including_this(std::string entry, bool to_warn, bool for_swap, bool to_remove)
{
    std::transform(entry.begin(), entry.end(), entry.begin(), ::tolower); // make sure capitalization does not matter in detecting matches (all lowercase)

    if (to_warn)
    {
        potential_duplicate_to_warn += entry + DELIMITER_CLEAN; // should have the entry come before the delimiter for duplicates to warn of
    }

    if (for_swap)
    {
        potential_swap += entry + DELIMITER_CLEAN; // should have the entry come before the delimiter for swapped() to behave properly
    }

    if (to_remove)
    {
        potential_duplicate_to_remove += DELIMITER_CLEAN + entry; // all potential duplicates to remove are tagged by the delimiter in the front!
    }
}

std::string swapped(std::string potential_duplicate)
{
    std::istringstream iss(potential_duplicate); // assumes entry comes before the delimiter in potential_duplicate
    std::string cell; // temporarily store entry
    std::getline(iss, cell, DELIMITER_CLEAN); // scan through potential_duplicate, stopping at the first delimiter
    std::string first_cell = cell; // store everything before the first delimiter as the first entry
    std::getline(iss, cell, DELIMITER_CLEAN); // starting after the first delimiter, scan until second delimiter is reached
    std::string second_cell = cell; // store everything between first and second delimiters as the second entry
    std::getline(iss, cell); // scan through the rest of potential_duplicate just in case there were other entries
    std::string other_cells = cell; // store everything else after the second delimiter
    
    std::string original_copy = potential_swap; // back up the actual potential_swap; will manipulate it to construct swapped version
    potential_swap.erase(); // since actual potential_swap is backed up, clear it up to store swapped version of row string
    track_duplicates_including_this(second_cell, false, true, false); // store second entry now as the first entry
    track_duplicates_including_this(first_cell, false, true, false); // store the first entry now as the second entry
    std::string swapped_copy = potential_swap + other_cells; // add on the other entries (swapped() only swaps first two entries)
    potential_swap = original_copy; // restore the actual potential_swap from backup
    return swapped_copy; // this is the swapped version of the row string
}

/* Functions for CreateLogFile() */
std::string add_log(std::string log)
{
    logs.push_back(log);

    return log; // return the original log string for convenience of printing messages to console
}

/* Functions for parameter initialization upon program start */
/* Express-Available Parameters */
std::string init_NAME_INPUT()
{
    const std::string default_name = "inputForCardea";

    std::string name;
    std::cout << "What is the name of the input file? (.csv will be automaticaly appended)" 
        << ((MODE == "EXPRESS") ? "" : " (NAME_INPUT)") << std::endl; // include parameter name when NOT in express mode
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
    std::cout << "How should the output file be named? (.csv will be automaticaly appended)" 
        << ((MODE == "EXPRESS") ? "" : " (NAME_OUTPUT)") << std::endl; // include parameter name when NOT in express mode
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
    std::cout << "What is the screening name according to the consent form file name?"
        << ((MODE == "EXPRESS") ? "" : " (NAME_EVENT)") << std::endl; // include parameter name when NOT in express mode
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
    const std::string default_name = "C:\\Users\\Bryan\\SHF\\Heart Screenings\\Forms"; // escaped characters; C:\Users\Bryan\SHF\Heart Screenings\Forms
    
    std::cout << "What is the path to the consent forms?"
        << ((MODE == "EXPRESS") ? "" : " (NAME_FORM_PATH)") << std::endl; // include parameter name when NOT in express mode
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
            else // path is not valid; will be prompted for path name again until a valid path is given
            {
                std::cout << "Path name is invalid. Check for typos or if path exists. Slash direction does not matter." << std::endl;
            }
        }
    }
}

/* Custom-Only Parameters */
std::string init_FORM_YES()
{
    const std::string default_yes = "Yes";

    if (MODE == "EXPRESS") return default_yes; // initialize advanced parameter to default value in express mode

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
    
    if (MODE == "EXPRESS") return default_no; // initialize advanced parameter to default value in express mode

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

    if (MODE == "EXPRESS") return default_name; // initialize advanced parameter to default value in express mode

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

    if (MODE == "EXPRESS") return default_name; // initialize advanced parameter to default value in express mode

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

    if (MODE == "EXPRESS") return default_name; // initialize advanced parameter to default value in express mode

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

    if (MODE == "EXPRESS") return default_name; // initialize advanced parameter to default value in express mode

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

    if (MODE == "EXPRESS") return default_delimiter; // initialize advanced parameter to default value in express mode

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

    if (MODE == "EXPRESS") return default_delimiter; // initialize advanced parameter to default value in express mode

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

/* Function for Advanced Setup */
std::string init_MODE()
{
    /*
        ASCII text art was generated from or adapted from https://patorjk.com/software/taag/.
        The Saving Hearts Foundation logo was adapted from http://loveascii.com/hearts.html.
    */
    std::cout << R"(******************************************************************************************
******************************************************************************************
                    _                                   _         
                   | |                                 | |        
    __      __ ___ | |  ___   ___   _ __ ___    ___    | |_   ___  
    \ \ /\ / // _ \| | / __| / _ \ | '_ ` _ \  / _ \   | __| / _ \ 
     \ V  V /|  __/| || (__ | (_) || | | | | ||  __/   | |_ | (_) |
      \_/\_/  \___||_| \___| \___/ |_| |_| |_| \___|    \__| \___/ 

        ....           ....         _____            _____   _____   ______           
      .::::::::     ::::::::.      / ____|    /\    |  __ \ |  __ \ |  ____|    /\    
    .:  *** ::::: ::::: ***  :.   | |        /  \   | |__) || |  | || |__      /  \   
       ****** ::::::: ******      | |       / /\ \  |  _  / | |  | ||  __|    / /\ \  
    :  *********:::*********  :   | |____  / ____ \ | | \ \ | |__| || |____  / ____ \ 
     :: *********:********* ::     \_____|/_/    \_\|_|  \_\|_____/ |______|/_/    \_\
      ::: *************** :::      _____          _____    _____  ______  _____     _ 
         :::***********:::        |  __ \  /\    |  __ \  / ____||  ____||  __ \   | |
            ::*******::           | |__) |/  \   | |__) || (___  | |__   | |__) |  | |
              ::***::             |  ___// /\ \  |  _  /  \___ \ |  __|  |  _  /   | |
                :*:               | |   / ____ \ | | \ \  ____) || |____ | | \ \   |_|
                 :                |_|  /_/    \_\|_|  \_\|_____/ |______||_|  \_\  (_)


******************************************************************************************
******************************************************************************************
        )"<< std::endl;
    std::cout << "Welcome to the Saving Hearts Foundation Cardea Parser!\n\n" 
        "Press ENTER to start this program." << std::endl;

    std::string mode; // stores specified mode
    while (std::getline(std::cin, mode))
    {
        std::transform(mode.begin(), mode.end(), mode.begin(), ::toupper); // make sure capitalization does not matter for input (all uppercase)
        
        if (mode.empty()) // implicit express mode; advanced parameters are automatically initialized to program defaults
        {
            mode = "EXPRESS"; // explicitly set mode to be express
            break;
        }
        else if (mode == "EXPRESS") // explicit express mode; visibly indicate fast initialization in text file instead of just blank line
        {
            break;
        }
        else if (mode == "CUSTOM") // custom mode; gives access to initializing advanced parameters from program interface
        {
            std::cout << "Custom mode activated." << std::endl;
            break;
        }
        else if (mode == "RETAIN") // custom mode but retains intermediate files; most likley used for debugging purposes
        {
            std::cout << "Custom mode activated. Retaining intermediate files." << std::endl;
            break;
        }
        else if (mode == "SHF")
        {
            std::cout << R"(
                                      __________ ___      ___ ___________   ___ 
        ....           ....          /          |   |    |   |           | |   |
      .::::::::     ::::::::.       /   ________|   |    |   |    _______| |   |
    .:  *** ::::: ::::: ***  :.    |   /        |   |    |   |   |         |   |
       ****** ::::::: ******       |  |         |   |    |   |   |         |   |
    :  *********:::*********  :    |   \______  |   |____|   |   |_______  |   |
     :: *********:********* ::      \         \ |            |           | |   |
      ::: *************** :::        \______   \|    ____    |    _______| |   |
         :::***********:::                  \   |   |    |   |   |         |   |
            ::*******::                      |  |   |    |   |   |         |___|
              ::***::               ________/   |   |    |   |   |          ___
                :*:                |           /|   |    |   |   |         /   \
                 :                 |__________/ |___|    |___|___|         \___/

                )" << std::endl;

            std::cout << "Press ENTER to start this program." << std::endl;
        }
        else if (mode == "HEART")
        {
            std::cout << R"(
                          |  \ \ | |/ /
                          |  |\ `' ' /
                          |  ;'aorta \      / , pulmonary
                          | ;    _,   |    / / ,  arteries
                 superior | |   (  `-.;_,-' '-' ,
                vena cava | `,   `-._       _,-'_
                          |,-`.    `.)    ,<_,-'_, pulmonary
                         ,'    `.   /   ,'  `;-' _,  veins
                        ;        `./   /`,    \-'
                        | right   /   |  ;\   |\
                        | atrium ;_,._|_,  `, ' \
                        |        \    \ `       `,
                        `      __ `    \   left  ;,
                         \   ,'  `      \,  ventricle
                          \_(            ;,      ;;
                          |  \           `;,     ;;
                 inferior |  |`.          `;;,   ;'
                vena cava |  |  `-.        ;;;;,;'
                          |  |    |`-.._  ,;;;;;'
                          |  |    |   | ``';;;'  FL
                                  aorta
                )" << std::endl;

            std::cout << "ASCII art from <http://loveascii.com/hearts.html>. \n\n"
                "Press ENTER to start this program." << std::endl;
        }
        else
        {
            /* Program does not continue until a valid mode is entered verbatim or nothing is entered */
            std::cout << "Input not recognized. Press ENTER to start this program." << std::endl;
        }
    }

    return mode; // mode set for the rest of this program
}