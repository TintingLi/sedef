/// 786 

#include <bits/stdc++.h>
#include <edlib.h>
#include "common.h"
#include "search.h"
using namespace std;

extern int QGRAM_NORMAL_FAILED;
extern int QGRAM_SPACED_FAILED;
extern int EDIT_FAILED;

inline void TIME(string msg, auto &start)
{
    auto end = chrono::high_resolution_clock::now();
    eprn("{}: {:.2f}s", 
        msg, 
        chrono::duration_cast<chrono::milliseconds>(end - start).count() / 1000.00);
    start = end;
}

int main(int argc, char **argv)
{
    if (argc < 3) exit(1);

    auto edlib_conf =  edlibDefaultAlignConfig();
    edlib_conf.mode = EDLIB_MODE_HW;

    eprn("🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁", 0);
    eprn("   🐚  S 🐚  E 🐚  D 🐚  E 🐚  F 🐚  ", 0);
    eprn("🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁 🍁", 0);
    eprnn("   🖥  {}; arguments: ", GITVER);
    for (int i = 0; i < argc; i++) eprnn(" {}", argv[i]);
    eprnn("\n");
    
    auto t_start = chrono::high_resolution_clock::now();

    string ref_path = argv[1];
    string query_path = argv[2];
    bool is_complement = (argc > 3 && tolower(argv[3][0]) == 'y');

    string line, reference, query;

    ifstream fin;

    fin.open(ref_path.c_str());
    string ref_chr;
    while (getline(fin, line)) {
        if (line[0] != '>') reference += line;
        else ref_chr = line.substr(1);
    }
    fin.close();
    fin.clear();

    transform(reference.begin(), reference.end(), reference.begin(), ::toupper);
    if (is_complement) {
        reverse(reference.begin(), reference.end());
        for (auto &c: reference) c = (c == 'A' ? 'T' : (c == 'C' ? 'G' : (c == 'G' ? 'C' : (c == 'T' ? 'A' : c))));
    }

    fin.open(query_path.c_str());
    string query_chr;
    while (getline(fin, line)) {
        if (line[0] != '>') query += line;
        else query_chr = line.substr(1);
    }
    fin.close();
    transform(query.begin(), query.end(), query.begin(), ::toupper);
        TIME("Reading time", t_start);

    auto ref_hash = Hash(reference);
        TIME("Reference hashing time", t_start);

    /// TODO optimize if ref == query
    // query = query.substr(0, 10000000);
    auto &query_hash = ref_hash; 
    Hash _q;
    if (query_path != ref_path || query.size() != reference.size() || is_complement) {
        _q = Hash(query);
        query_hash = _q;
    }

    bool allow_overlaps = (ref_path != query_path) || is_complement;
    eprn("Allowing overlaps: {}", allow_overlaps);

    int total = 0;
    for (int i = 0, j = 0; i < query.size(); i += 250, j++) {
    // for (int i = 55002548; i <= 55002548; i++) {
        while (query[i] == 'N') i++;
        while (i % 250 != 0) i++;
        if (i % 5000 == 0) {
            double perc = 100.0 * i / double(query.size());
            eprnn("\r   {} {:.1f}% ({})", string(int(perc / 2) + 1, '-'), perc, i);
        }
        // eprnn("---\n");

        auto mapping = search(i, ref_hash, query_hash, allow_overlaps);
        //    TIME("Mapping time", t_start);

        // prn("{} mappings in total", mapping.size()); //3654?
        for (auto &pp: mapping) /*if (p.second > .5)*/ {    
            // BEDPE
            if (is_complement) {
                pp.i = ref_hash.seq.size() - pp.i + 1;
                pp.j = ref_hash.seq.size() - pp.j + 1;
                swap(pp.i, pp.j);
            }
            //if (pp.init_id < 95) continue;
            // prn("---{}", pp.matches.size());
            prn("{}\t{}\t{}\t{}\t{}\t{}\t\t{:.0f}\t{}\t{}\t{}\t{}",
                query_chr, pp.p, pp.q, 
                ref_chr, pp.i, pp.j,
                pp.id, 
                "+", is_complement ? "-" : "+",
                // Optional fields
                int(pp.break_criteria),
                pp.q - pp.p //, pp.j - pp.i,
                // pp.init_id,
                // pp.jaccard.first, pp.jaccard.second
            );
            // edlib_conf.k = max(pp.q - pp.p, pp.j - pp.i) / 2;
            // auto result = edlibAlign(
            //     query_hash.seq.c_str() + pp.p, pp.q - pp.p,
            //     ref_hash.seq.c_str() + pp.i, pp.j - pp.i,
            //     edlib_conf
            // );
            // char* cigar = edlibAlignmentToCigar(result.alignment, result.alignmentLength, EDLIB_CIGAR_STANDARD);
            // prn("{}\t{}", result.editDistance, cigar);
            // free(cigar);

            total += 1;

            // for (auto &m: pp.matches)
                // prnn("{},{};", m.first, m.second);
            // prnn("\n");
        }
        // if(mapping.size())    exit(0);
        //break;
        // prnn("\n");
    }
    eprnn("\n");

    eprn("Total:               {:10n}", total);
    eprn("Fails: edit          {:10n}\n"
         "       q-gram normal {:10n}\n"
         "       q-gram spaced {:10n}", EDIT_FAILED, QGRAM_NORMAL_FAILED, QGRAM_SPACED_FAILED);

    return 0;
}