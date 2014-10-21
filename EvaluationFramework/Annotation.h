#pragma once
#include <string>
#include <vector>

namespace EvaluationFramework
{
	using namespace std;


	class Annotation
	{
		int id;
	public:
		Annotation(int _id)
			: id(_id) {}

		virtual void fit(const vector<double>&) {};

		virtual double overlap(const Annotation&)=0;
	};

	namespace BoundingVolume
	{
		class Box : public Annotation
		{
			double min_x, min_y, max_x, max_y;

		public:
			Box(int id=0, double _min_x=0.0,  double _min_y=0.0,  double _max_x=0.0, double _max_y=0.0)
				: min_x(_min_x), min_y(_min_y), max_x(_max_x), max_y(_max_y), Annotation(id) 
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

	class Frame
	{
		int id;
		string image_name;
		vector<const Annotation*> annotations;

	public:
		void Add(const Annotation* annotation)
		{			
			annotations.push_back(annotation);
		}
	};
}