#include "Annotation.h"

#include <iostream>
#include <functional>
#include <numeric>

using namespace EvaluationFramework;
using namespace std;

template<typename T>
std::ostream& operator<<(std::ostream& stm, const std::vector<T>& obj) {
	stm << "[";
	if (!obj.empty()) {
		for (size_t i = 0 ; i < obj.size()-1 ; ++i) {
			stm << obj[i] << " ";
		}
		stm << obj.back();
	}
	stm << "]";
	return stm;
}

template< class T >
void reorder(vector<T> &v, vector<size_t> const &order )  
{   
    for ( unsigned int s = 1, d; s < order.size(); ++ s ) {
    	for ( d = order[s]; d < s; d = order[d] ) ;
    	if ( d == s ) while ( d = order[d], d != s ) swap( v[s], v[d] );
    }
}

template< class T >
void reorder_naive(vector<T>& vA, const vector<size_t>& vOrder)  
{   
    vector<T> vCopy = vA;
    for(unsigned int i = 0; i < vOrder.size(); ++i)  
        vA[i] = vCopy[ vOrder[i] ];  
}

template <typename T>
vector<size_t> sort_indexes(const vector<T> &v) {

  // initialize original index locations
  vector<size_t> idx(v.size());
  for (size_t i = 0; i != idx.size(); ++i) idx[i] = i;

  // sort indexes based on comparing values in v
  sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

  return idx;
}

int main()
{
	vector<Frame> gt, output;
	LoadPascal(gt, "gt.txt");
	LoadPascal(output, "output.txt");
	double eval_overlap = 0.5;

	vector< vector<double>> tpc(gt.size());
	vector< vector<int>> tp(gt.size());
	vector< vector<int>> fp(gt.size());
	vector< vector<int>> fn(gt.size());
	vector< vector<double>> weights(gt.size());

	for (unsigned int k = 0; k < gt.size(); k++)
	{
		//sort the detection in descending order
		output[k].Sort();

		//calculate overlaps between gt and output
		vector< vector<double> > overlap(gt[k].annotations.size(), vector<double>(output[k].annotations.size(),0.0));

		for (unsigned int i = 0; i < gt[k].annotations.size(); i++)
			for (unsigned int j = 0; j < output[k].annotations.size(); j++)
				overlap[i][j] = gt[k].annotations[i]->overlap(*output[k].annotations[j]);

		//calculate assignments between gt and output
		vector<int> assigned(output[k].annotations.size(), -1);

		for (unsigned int i = 0; i < overlap.size(); i++)
		{
			double max_overlap = eval_overlap;
			int index = -1;

			for (unsigned int j = 0; j < overlap[i].size(); j++)
			{
				if ((assigned[j] == -1) && (overlap[i][j] > max_overlap))
				{
					index = j;
					max_overlap = overlap[i][j];
				}
			}
			if (index != -1)
			{
				assigned[index] = i;
			}
		}

		tpc[k] = vector<double>(assigned.size(), 0.0);
		tp[k] = vector<int>(assigned.size(), 0);
		fp[k] = vector<int>(assigned.size(), 0);
		fn[k] = vector<int>(assigned.size(), 0);
		weights[k] = vector<double>(assigned.size(), 0);

		double prev_tpc = 0;
		int prev_tp = 0;
		int prev_fp = 0;
		int prev_fn = 0;

		//count
		for (unsigned int i = 0; i < output[k].annotations.size(); i++)
		{
			for (unsigned int j = 0; j < assigned.size(); j++)
			{
				if (output[k].annotations[j]->weight >= output[k].annotations[i]->weight)
				{
					//positives
					if (assigned[j] != -1)
					{
						//true positives
						tp[k][i]++;
						tpc[k][i] += overlap[assigned[j]][j];
					}
					else
					{
						//false positive
						fp[k][i]++;
					}
				}
				else
					break;
			}
			//calculate difference between tp and total gt - these are all missed detections
			fn[k][i] = gt[k].annotations.size() - tp[k][i];

			//store just differences
			tpc[k][i] -= prev_tpc;
			tp[k][i] -= prev_tp;
			fp[k][i] -= prev_fp;

			prev_tpc += tpc[k][i];
			prev_tp += tp[k][i];
			prev_fp += fp[k][i];

			weights[k][i] = output[k].annotations[i]->weight;
		}

//		cerr << "FN: " << fn[k] << endl;
		//false negatives need to be counted in reverse
		for (unsigned int i = fn[k].size()-1; i-- > 0;)
		{
			fn[k][i] -= prev_fn;
			prev_fn += fn[k][i];
		}

		if (0)
		{

			cerr << "Overlaps: ";
			for (unsigned int i = 0; i < gt[k].annotations.size(); i++)
			{
				for (unsigned int j = 0; j < output[k].annotations.size(); j++)
					cerr << overlap[i][j] << " ";
				cerr << endl;
			}

			cerr << "Weights: ";
			for (unsigned int i = 0; i < output[k].annotations.size(); i++)
				cerr << output[k].annotations[i]->weight << " ";
			cerr << endl;

			cerr << "ASS: " << assigned << endl;
			cerr << "TPC: " << tpc[k] << endl;
			cerr << "TP : " << tp[k] << endl;
			cerr << "FP : " << fp[k] << endl;
			cerr << "FN : " << fn[k] << endl;
		}
	}

	//concatenate all metrics
	vector<double> weights_all;
	vector<double> tpc_all;
	vector<int> tp_all;
	vector<int> fp_all;
	vector<int> fn_all;
	for (unsigned int i = 0; i < gt.size(); i++)
	{
		weights_all.insert(weights_all.end(), weights[i].begin(), weights[i].end());
		tpc_all.insert(tpc_all.end(),tpc[i].begin(),tpc[i].end());
		tp_all.insert(tp_all.end(),tp[i].begin(),tp[i].end());
		fp_all.insert(fp_all.end(),fp[i].begin(),fp[i].end());
		fn_all.insert(fn_all.end(),fn[i].begin(),fn[i].end());
	}

	vector<size_t> sorted_ind = sort_indexes(weights_all);

	reorder_naive(weights_all, sorted_ind);
	reorder_naive(tpc_all, sorted_ind);
	reorder_naive(tp_all, sorted_ind);
	reorder_naive(fp_all, sorted_ind);
	reverse(fn_all.begin(), fn_all.end());
	reorder_naive(fn_all, sorted_ind);

	std::partial_sum(tpc_all.begin(), tpc_all.end(), tpc_all.begin());
	std::partial_sum(tp_all.begin(), tp_all.end(), tp_all.begin());
	std::partial_sum(fp_all.begin(), fp_all.end(), fp_all.begin());
	std::partial_sum(fn_all.begin(), fn_all.end(), fn_all.begin());
	reverse(fn_all.begin(), fn_all.end());

	cerr << "weights: " << weights_all << endl;
	cerr << "TPC: " << tpc_all << endl;
	cerr << "TP : " << tp_all << endl;
	cerr << "FP : " << fp_all << endl;
	cerr << "FN : " << fn_all << endl;

	return 0;
}
