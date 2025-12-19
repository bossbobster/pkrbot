// ORIGINAL IMPLEMENTATION IN C++ (new one is in cython)

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <algorithm>
#include <vector>
namespace py = pybind11;
using namespace std;

constexpr uint16_t straight_masks[10] = {0b1111100000000, 0b0111110000000, 0b0011111000000, 0b0001111100000, 0b0000111110000, 0b0000011111000, 0b0000001111100, 0b0000000111110, 0b0000000011111, 0b1000000001111};

// evaluate best 5-card hand from n cards in just one go
uint32_t evaluate(py::array_t<uint8_t>& ranks_in, py::array_t<uint8_t>& suits_in) 
{
    auto ranks = ranks_in.unchecked<1>();
    auto suits = suits_in.unchecked<1>();
    uint8_t n = static_cast<uint8_t>(ranks.shape(0));
    assert(n >= 5 && n <= 52 && "Need between 5 and 52 cards to evaluate");

    vector<uint8_t> counts_r(13, 0);
    vector<uint8_t> counts_s(4, 0);
    uint16_t overall = 0, persuit[4] = {0, 0, 0, 0}, mxfl = 0;

    for(uint8_t i = 0; i < n; i ++)
    {
        counts_r[ranks(i)] ++;
        counts_s[suits(i)] ++;
        overall |= (1u << ranks(i));
        persuit[suits(i)] |= (1u << ranks(i));
    }

    bool is_flush = false, straight_flush = false;
    uint8_t strfl_mx = 0, mxfl_cnt = 0;
    for(uint8_t s = 0; s < 4; s ++) 
        if(counts_s[s] >= 5)
        { 
            is_flush = true; 
            mxfl = max(mxfl, persuit[s]);
            mxfl_cnt = counts_s[s];
            for(uint8_t r = 0; r < 10; r ++)
                if((persuit[s] & straight_masks[r]) == straight_masks[r])
                {
                    strfl_mx = max(strfl_mx, (uint8_t)(10-r));
                    straight_flush = true;
                    break;
                }
        }
    if(straight_flush) return (9 << 20) | (strfl_mx << 16);

    int mx1 = -1, mx2 = -1, mxp1 = -1, mxp2 = -1, mxq = -1, mxt = -1;
    for(uint8_t r = 0; r < 13; r ++)
    {
        if(counts_r[r] == 4) mxq = r;
        else if(counts_r[r] == 3)
        {
            mxp1 = max(mxp1, mxt);
            mxt = r;
        }
        else if(counts_r[r] == 2)
        {
            if(mxp2 > mx1) { mx2 = mx1; mx1 = mxp2; }
            else if(mxp2 > mx2) mx2 = mxp2;
            mxp2 = mxp1;
            mxp1 = r;
        }
        else if(counts_r[r] == 1)
        {
            mx2 = mx1;
            mx1 = r;
        }
    }

    // quads
    if(mxq != -1) return (8 << 20) | (mxq << 16) | (max({mx1, mxp1, mxt}) << 12);

    // full house
    if(mxt != -1 && mxp1 != -1) return (7 << 20) | (mxt << 16) | (mxp1 << 12);

    // flush
    if(is_flush)
    {
        if(mxfl_cnt > 5)
            for(uint8_t r = 0; mxfl_cnt > 5; r ++)
            {
                if(mxfl & (1u << r))
                {
                    mxfl_cnt --;
                    mxfl &= ~(1u << r);
                }
            }
        return (6 << 20) | mxfl;
    }

    // straight
    for(uint8_t r = 0; r < 10; r ++)
        if((overall & straight_masks[r]) == straight_masks[r])
            return (5 << 20) | ((10-r) << 16);

    // trips (no pairs left, so just take max trip and two max singles)
    if(mxt != -1) return (4 << 20) | (mxt << 16) | (mx1 << 12) | (mx2 << 8);

    // two pair (no trips, just mxp1, mxp2, and one mx1)
    if(mxp2 != -1) return (3 << 20) | (mxp1 << 16) | (mxp2 << 12) | (mx1 << 8);

    // one pair (no trips or two pair, just mx1 and mx2)
    if(mxp1 != -1) 
    {
        // gotta search for mx3
        int mx3 = -1;
        for(uint8_t r = mx2-1; r >= 0; r --)
            if(counts_r[r] && r != mxp1)
            {
                mx3 = r;
                break;
            }
        return (2 << 20) | (mxp1 << 16) | (mx1 << 12) | (mx2 << 8) | (mx3 << 4);
    }

    // high card (no pairs)
    if(n > 5)
        for(uint8_t r = 0; n > 5; r ++)
            if(overall & (1u << r))
            {
                n --;
                overall &= ~(1u << r);
            }
    return (1 << 20) | overall;
}

PYBIND11_MODULE(pkrbot, m) 
{
    // m.def("embed_5card", &embed_5card, "Super fast 5-card embed");
    // m.def("evaluate", &evaluate, "Evaluate best 5-card hand from N cards");
    // m.def("eval", &eval, "Evaluate best 5-card hand from N cards");
    m.def("evaluate", &evaluate, "Evaluate best 5-card hand from N cards");
}







































// // n = hands.shape[0]
// py::array_t<uint32_t> embed_5card(py::array_t<uint8_t> ranks_in, py::array_t<uint8_t> suits_in) 
// {
//     auto ranks  = ranks_in.unchecked<2>(); // shape (n,5)
//     auto suits  = suits_in.unchecked<2>();
//     int64_t n = ranks.shape(0);
//     py::array_t<uint32_t> out(n);
//     auto out_mut = out.mutable_unchecked<1>();

//     // Precompute straight masks
//     uint16_t straight_masks[9];
//     for(int i = 0; i < 9; i ++) 
//         straight_masks[i] = ((1 << 5) - 1) << i;
//     const uint16_t wheel_mask = (1u << 12) | ((1u << 4) - 1);

//     vector<uint8_t> counts_r(13);
//     vector<uint8_t> counts_s(4);
//     vector<uint32_t> composite(13);

//     for(int64_t i = 0; i < n; i ++)
//     {
//         // zero counts
//         fill(counts_r.begin(), counts_r.end(), 0);
//         fill(counts_s.begin(), counts_s.end(), 0);

//         // 1) histograms
//         for(int j = 0; j < 5; j ++)
//             counts_r[ranks(i,j)] ++, counts_s[suits(i,j)] ++;

//         // 2) flush & straight
//         bool is_flush = false;
//         for(int s = 0; s < 4; s ++) 
//             if(counts_s[s] == 5)
//             { 
//                 is_flush = true; 
//                 break; 
//             }

//         // build bitmask
//         uint16_t mask = 0;
//         for(int r = 0; r < 13; r ++)
//             if(counts_r[r]) 
//                 mask |= (1u << r);

//         bool is_straight = false;
//         for(auto m : straight_masks)
//             if((mask & m) == m)
//             { 
//                 is_straight=true; 
//                 break; 
//             }
//         if(!is_straight && (mask & wheel_mask) == wheel_mask)
//             is_straight = true;

//         // 3) quads/trips/pairs
//         bool quad = false, trip=false;
//         int pair_count = 0;
//         for(int r = 0; r < 13; r ++)
//         {
//             if(counts_r[r] == 4) quad = true;
//             if(counts_r[r] == 3) trip = true;
//             if(counts_r[r] == 2) pair_count ++;
//         }
//         bool fullhouse = trip && (pair_count >= 1);
//         bool two_pair = (pair_count == 2);
//         bool one_pair = (pair_count == 1) && !fullhouse;

//         // 4) hand_type
//         uint32_t ht=0;
//         if(one_pair) ht=1;
//         if(two_pair) ht=2;
//         if(trip && !fullhouse) ht=3;
//         if(is_straight) ht=4;
//         if(is_flush) ht=5;
//         if(fullhouse) ht=6;
//         if(quad) ht=7;
//         if(is_straight && is_flush) ht=8;

//         // 5) Determine kickers based on hand type
//         uint32_t emb = ht << 20;
        
//         if(is_straight) {
//             // For straights, store only the high card of the straight
//             // Check if it's a wheel (A-2-3-4-5), which is 5-high
//             bool is_wheel = (mask & wheel_mask) == wheel_mask;
//             if(is_wheel) {
//                 // Wheel: high card is 5 (rank 3)
//                 emb |= (3 << 16);  // rank 3 = 5
//             } else {
//                 // Find highest rank in straight by checking mask bits
//                 int high_rank = 12;
//                 for(int r = 12; r >= 0; r--) {
//                     if(counts_r[r] > 0) {
//                         high_rank = r;
//                         break;
//                     }
//                 }
//                 emb |= (high_rank << 16);
//             }
//         } else {
//             // For non-straights, use composite key approach
//             for(int r = 0; r < 13; r ++)
//             {
//                 uint32_t c = (uint32_t(counts_r[r]) << 4) | uint32_t(r);
//                 composite[r] = (counts_r[r] > 0 ? c : 0);
//             }

//             // pick top 5 via partial sort
//             nth_element(composite.begin(), composite.begin()+5, composite.end(), greater<uint32_t>());
//             sort(composite.begin(), composite.begin()+5, greater<uint32_t>());

//             // extract kicker ranks
//             for(int k = 0; k < 5; k ++)
//                 emb |= ( (composite[k] & 0xF) << (16 - 4*k) );
//         }
        
//         out_mut(i) = emb;
//     }
//     return out;
// }

// constexpr int nchoose5[48] = {1, 6, 21, 56, 126, 252, 462, 792, 1287, 2002, 
//     3003, 4368, 6188, 8568, 11628, 15504, 20349, 26334, 33649, 
//     42504, 53130, 65780, 80730, 98280, 118755, 142506, 169911, 201376, 237336, 278256, 324632, 376992, 
//     435897, 501942, 575757, 658008, 749398, 850668, 962598, 1086008, 1221759, 1370754, 1533939, 1712304, 
//     1906884, 2118760, 2349060, 2598960};

// // Template to generate all combinations for a specific n_cards
// template<int N_CARDS>
// struct Combinations 
// {
//     static constexpr int N_COMBOS = nchoose5[N_CARDS - 5];
    
//     int data[N_COMBOS][5];
    
//     constexpr Combinations() : data{} 
//     {
//         int indices[5] = {0, 1, 2, 3, 4};
//         int combo_idx = 0;
        
//         // Store first combination
//         for(int i = 0; i < 5; i++)
//             data[combo_idx][i] = indices[i];
//         combo_idx++;
        
//         // Generate remaining combinations sequentially - O(N) total
//         while(combo_idx < N_COMBOS) 
//         {
//             // Find rightmost index that can be incremented
//             int i = 4;
//             while(i >= 0 && indices[i] == N_CARDS - 5 + i)
//                 i--;
            
//             if(i < 0) break;
            
//             // Increment and reset following indices
//             indices[i]++;
//             for(int j = i + 1; j < 5; j++)
//                 indices[j] = indices[j - 1] + 1;
            
//             // Store this combination
//             for(int k = 0; k < 5; k++)
//                 data[combo_idx][k] = indices[k];
//             combo_idx++;
//         }
//     }
// };

// // Precomputed at compile time
// constexpr Combinations<6> COMBOS_6; constexpr Combinations<7> COMBOS_7; constexpr Combinations<8> COMBOS_8; constexpr Combinations<9> COMBOS_9;
// constexpr Combinations<10> COMBOS_10; constexpr Combinations<11> COMBOS_11; constexpr Combinations<12> COMBOS_12; constexpr Combinations<13> COMBOS_13;
// constexpr Combinations<14> COMBOS_14; constexpr Combinations<15> COMBOS_15; constexpr Combinations<16> COMBOS_16; constexpr Combinations<17> COMBOS_17;
// // constexpr Combinations<18> COMBOS_18; constexpr Combinations<19> COMBOS_19; constexpr Combinations<20> COMBOS_20; constexpr Combinations<21> COMBOS_21;
// // constexpr Combinations<22> COMBOS_22; constexpr Combinations<23> COMBOS_23; constexpr Combinations<24> COMBOS_24; constexpr Combinations<25> COMBOS_25;
// // constexpr Combinations<26> COMBOS_26; constexpr Combinations<27> COMBOS_27; constexpr Combinations<28> COMBOS_28; constexpr Combinations<29> COMBOS_29;
// // constexpr Combinations<30> COMBOS_30; constexpr Combinations<31> COMBOS_31; constexpr Combinations<32> COMBOS_32; constexpr Combinations<33> COMBOS_33;
// // constexpr Combinations<34> COMBOS_34; constexpr Combinations<35> COMBOS_35; constexpr Combinations<36> COMBOS_36; constexpr Combinations<37> COMBOS_37;
// // constexpr Combinations<38> COMBOS_38; constexpr Combinations<39> COMBOS_39; constexpr Combinations<40> COMBOS_40; constexpr Combinations<41> COMBOS_41;
// // constexpr Combinations<42> COMBOS_42; constexpr Combinations<43> COMBOS_43; constexpr Combinations<44> COMBOS_44; constexpr Combinations<45> COMBOS_45;
// // constexpr Combinations<46> COMBOS_46; constexpr Combinations<47> COMBOS_47; constexpr Combinations<48> COMBOS_48; constexpr Combinations<49> COMBOS_49;
// // constexpr Combinations<50> COMBOS_50; constexpr Combinations<51> COMBOS_51; constexpr Combinations<52> COMBOS_52;

// // Inline 5-card evaluation (no array allocations)
// inline uint32_t evaluate_5cards_inline(const uint8_t* ranks, const uint8_t* suits) {
//     // Precompute straight masks
//     static constexpr uint16_t straight_masks[9] = {
//         0x001F, 0x003E, 0x007C, 0x00F8, 0x01F0, 0x03E0, 0x07C0, 0x0F80, 0x1F00
//     };
//     static constexpr uint16_t wheel_mask = (1u << 12) | ((1u << 4) - 1);
    
//     uint8_t counts_r[13] = {0};
//     uint8_t counts_s[4] = {0};
    
//     // 1) histograms
//     for(int j = 0; j < 5; j++) {
//         counts_r[ranks[j]]++;
//         counts_s[suits[j]]++;
//     }
    
//     // 2) flush & straight
//     bool is_flush = false;
//     for(int s = 0; s < 4; s++)
//         if(counts_s[s] == 5) {
//             is_flush = true;
//             break;
//         }
    
//     // build bitmask
//     uint16_t mask = 0;
//     for(int r = 0; r < 13; r++)
//         if(counts_r[r])
//             mask |= (1u << r);
    
//     bool is_straight = false;
//     for(auto m : straight_masks)
//         if((mask & m) == m) {
//             is_straight = true;
//             break;
//         }
//     if(!is_straight && (mask & wheel_mask) == wheel_mask)
//         is_straight = true;
    
//     // 3) quads/trips/pairs
//     bool quad = false, trip = false;
//     int pair_count = 0;
//     for(int r = 0; r < 13; r++) {
//         if(counts_r[r] == 4) quad = true;
//         if(counts_r[r] == 3) trip = true;
//         if(counts_r[r] == 2) pair_count++;
//     }
//     bool fullhouse = trip && (pair_count >= 1);
//     bool two_pair = (pair_count == 2);
//     bool one_pair = (pair_count == 1) && !fullhouse;
    
//     // 4) hand_type
//     uint32_t ht = 0;
//     if(one_pair) ht = 1;
//     if(two_pair) ht = 2;
//     if(trip && !fullhouse) ht = 3;
//     if(is_straight) ht = 4;
//     if(is_flush) ht = 5;
//     if(fullhouse) ht = 6;
//     if(quad) ht = 7;
//     if(is_straight && is_flush) ht = 8;
    
//     // 5) Determine kickers
//     uint32_t emb = ht << 20;
    
//     if(is_straight) {
//         bool is_wheel = (mask & wheel_mask) == wheel_mask;
//         if(is_wheel) {
//             emb |= (3 << 16);  // rank 3 = 5
//         } else {
//             int high_rank = 12;
//             for(int r = 12; r >= 0; r--) {
//                 if(counts_r[r] > 0) {
//                     high_rank = r;
//                     break;
//                 }
//             }
//             emb |= (high_rank << 16);
//         }
//     } else {
//         uint32_t composite[13];
//         for(int r = 0; r < 13; r++) {
//             uint32_t c = (uint32_t(counts_r[r]) << 4) | uint32_t(r);
//             composite[r] = (counts_r[r] > 0 ? c : 0);
//         }
        
//         // partial sort to get top 5
//         nth_element(composite, composite + 5, composite + 13, greater<uint32_t>());
//         sort(composite, composite + 5, greater<uint32_t>());
        
//         for(int k = 0; k < 5; k++)
//             emb |= ((composite[k] & 0xF) << (16 - 4*k));
//     }
    
//     return emb;
// }

// // Fast template evaluator using precomputed combinations
// template<int N_CARDS>
// uint32_t evaluate_with_combos(const Combinations<N_CARDS>& combos, py::array_t<uint8_t> ranks_in, py::array_t<uint8_t> suits_in)
// {
//     auto ranks = ranks_in.unchecked<1>();
//     auto suits = suits_in.unchecked<1>();
    
//     constexpr int64_t n_combos = Combinations<N_CARDS>::N_COMBOS;
    
//     uint32_t best = 0;
//     uint8_t hand_ranks[5];
//     uint8_t hand_suits[5];
    
//     // Evaluate each combination directly without intermediate storage
//     for(int64_t combo_idx = 0; combo_idx < n_combos; combo_idx++) {
//         // Extract this combination
//         for(int i = 0; i < 5; i++) {
//             hand_ranks[i] = ranks(combos.data[combo_idx][i]);
//             hand_suits[i] = suits(combos.data[combo_idx][i]);
//         }
        
//         // Evaluate inline
//         uint32_t score = evaluate_5cards_inline(hand_ranks, hand_suits);
//         if(score > best)
//             best = score;
//     }
    
//     return best;
// }

// uint32_t evaluate(py::array_t<uint8_t> ranks_in, py::array_t<uint8_t> suits_in) 
// {
//     auto ranks = ranks_in.unchecked<1>();
//     int64_t n_cards = ranks.shape(0);
    
//     assert(n_cards >= 5 && n_cards <= 52 && "Need between 5 and 52 cards to evaluate");
    
//     // For exactly 5 cards, evaluate directly
//     if(n_cards == 5) 
//     {
//         auto suits = suits_in.unchecked<1>();
//         uint8_t hand_ranks[5], hand_suits[5];
//         for(int i = 0; i < 5; i++) {
//             hand_ranks[i] = ranks(i);
//             hand_suits[i] = suits(i);
//         }
//         return evaluate_5cards_inline(hand_ranks, hand_suits);
//     }
    
//     // Dispatch to appropriate precomputed combination table
//     switch(n_cards) 
//     {
//         case 6:  return evaluate_with_combos(COMBOS_6,  ranks_in, suits_in);
//         case 7:  return evaluate_with_combos(COMBOS_7,  ranks_in, suits_in);
//         case 8:  return evaluate_with_combos(COMBOS_8,  ranks_in, suits_in);
//         case 9:  return evaluate_with_combos(COMBOS_9,  ranks_in, suits_in);
//         case 10: return evaluate_with_combos(COMBOS_10, ranks_in, suits_in);
//         case 11: return evaluate_with_combos(COMBOS_11, ranks_in, suits_in);
//         case 12: return evaluate_with_combos(COMBOS_12, ranks_in, suits_in);
//         case 13: return evaluate_with_combos(COMBOS_13, ranks_in, suits_in);
//         case 14: return evaluate_with_combos(COMBOS_14, ranks_in, suits_in);
//         case 15: return evaluate_with_combos(COMBOS_15, ranks_in, suits_in);
//         case 16: return evaluate_with_combos(COMBOS_16, ranks_in, suits_in);
//         case 17: return evaluate_with_combos(COMBOS_17, ranks_in, suits_in);
//         // case 18: return evaluate_with_combos(COMBOS_18, ranks_in, suits_in);
//         // case 19: return evaluate_with_combos(COMBOS_19, ranks_in, suits_in);
//         // case 20: return evaluate_with_combos(COMBOS_20, ranks_in, suits_in);
//         // case 21: return evaluate_with_combos(COMBOS_21, ranks_in, suits_in);
//         // case 22: return evaluate_with_combos(COMBOS_22, ranks_in, suits_in);
//         // case 23: return evaluate_with_combos(COMBOS_23, ranks_in, suits_in);
//         // case 24: return evaluate_with_combos(COMBOS_24, ranks_in, suits_in);
//         // case 25: return evaluate_with_combos(COMBOS_25, ranks_in, suits_in);
//         // case 26: return evaluate_with_combos(COMBOS_26, ranks_in, suits_in);
//         // case 27: return evaluate_with_combos(COMBOS_27, ranks_in, suits_in);
//         // case 28: return evaluate_with_combos(COMBOS_28, ranks_in, suits_in);
//         // case 29: return evaluate_with_combos(COMBOS_29, ranks_in, suits_in);
//         // case 30: return evaluate_with_combos(COMBOS_30, ranks_in, suits_in);
//         // case 31: return evaluate_with_combos(COMBOS_31, ranks_in, suits_in);
//         // case 32: return evaluate_with_combos(COMBOS_32, ranks_in, suits_in);
//         // case 33: return evaluate_with_combos(COMBOS_33, ranks_in, suits_in);
//         // case 34: return evaluate_with_combos(COMBOS_34, ranks_in, suits_in);
//         // case 35: return evaluate_with_combos(COMBOS_35, ranks_in, suits_in);
//         // case 36: return evaluate_with_combos(COMBOS_36, ranks_in, suits_in);
//         // case 37: return evaluate_with_combos(COMBOS_37, ranks_in, suits_in);
//         // case 38: return evaluate_with_combos(COMBOS_38, ranks_in, suits_in);
//         // case 39: return evaluate_with_combos(COMBOS_39, ranks_in, suits_in);
//         // case 40: return evaluate_with_combos(COMBOS_40, ranks_in, suits_in);
//         // case 41: return evaluate_with_combos(COMBOS_41, ranks_in, suits_in);
//         // case 42: return evaluate_with_combos(COMBOS_42, ranks_in, suits_in);
//         // case 43: return evaluate_with_combos(COMBOS_43, ranks_in, suits_in);
//         // case 44: return evaluate_with_combos(COMBOS_44, ranks_in, suits_in);
//         // case 45: return evaluate_with_combos(COMBOS_45, ranks_in, suits_in);
//         // case 46: return evaluate_with_combos(COMBOS_46, ranks_in, suits_in);
//         // case 47: return evaluate_with_combos(COMBOS_47, ranks_in, suits_in);
//         // case 48: return evaluate_with_combos(COMBOS_48, ranks_in, suits_in);
//         // case 49: return evaluate_with_combos(COMBOS_49, ranks_in, suits_in);
//         // case 50: return evaluate_with_combos(COMBOS_50, ranks_in, suits_in);
//         // case 51: return evaluate_with_combos(COMBOS_51, ranks_in, suits_in);
//         // case 52: return evaluate_with_combos(COMBOS_52, ranks_in, suits_in);
//         default:
//             assert(false && "Unreachable: n_cards already validated to be in [5,52]");
//             return 0;
//     }
// }