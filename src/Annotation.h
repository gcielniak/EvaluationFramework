#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <fstream>

namespace EvaluationFramework
{
	using namespace std;

	class Annotation
	{
		int id;

	public:
		double weight;

		Annotation(int _id, double _weight)
			: id(_id), weight(_weight) {}

		virtual void fit(const vector<double>&) {};

		virtual double overlap(const Annotation&)=0;
	};

	namespace BoundingVolume
	{
		class Box : public Annotation
		{
			double min_x, min_y, max_x, max_y;

		public:
			Box(int id=0, double _min_x=0.0,  double _min_y=0.0,  double _max_x=0.0, double _max_y=0.0, double weight=std::numeric_limits<double>::min())
				: min_x(_min_x), min_y(_min_y), max_x(_max_x), max_y(_max_y), Annotation(id, weight) 
			{
			}

			virtual double overlap(const Annotation& annotation)
			{
				const Box* box = dynamic_cast<const Box*>(&annotation);
				double overlap_ratio = 0.0;

				double dx = max(0.0,min(max_x, box->max_x)-max(min_x, box->min_x));
				if (dx > 0.0)
				{
					double dy = max(0.0,min(max_y, box->max_y)-max(min_y, box->min_y));
					if (dy > 0.0)
					{
						overlap_ratio = dx*dy;
						overlap_ratio = overlap_ratio/((max_x-min_x)*(max_y-min_y)+(box->max_x-box->min_x)*(box->max_y-box->min_y)-overlap_ratio);
					}
				}

				return overlap_ratio;
			}
		};
	}

	bool asc_order(const Annotation* l, const Annotation* r)
		{ return l->weight < r->weight; }

	bool dsc_order(const Annotation* l, const Annotation* r)
		{ return l->weight > r->weight; }

	class Frame
	{
		int id;

	public:
		vector<Annotation*> annotations;
		string image_name;

		void Add(Annotation* annotation)
		{			
			annotations.push_back(annotation);
		}

		void Sort()
		{
			std::sort(annotations.begin(), annotations.end(), dsc_order);
		}
	};

	void LoadPascal(vector<Frame>& frames, string file_name)
	{
		std::ifstream in(file_name);
		std::string line;
		string frame_id;
		double score, min_x, min_y, max_x, max_y;

		while (std::getline(in, line))
		{
			std::istringstream(line) >> frame_id >> score >> min_x >> min_y >> max_x >> max_y;
			Frame* cur_frame = 0;
			for (unsigned int i = 0; i < frames.size(); i++)
			{
				if (!frames[i].image_name.compare(frame_id))
					cur_frame = &frames.at(i);
			}
			
			if (!cur_frame)
			{
				frames.push_back(Frame());
				cur_frame = &frames.back();
				cur_frame->image_name = frame_id;
			}

			cur_frame->Add(new BoundingVolume::Box(0,min_x, min_y, max_x, max_y, score));
		}

		in.close();
	}
}