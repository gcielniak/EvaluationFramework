#include "Annotation.h"

using namespace EvaluationFramework;

int main()
{
	BoundingVolume::Box box;
	Frame frame;

	frame.Add(&box);
	frame.Add(new BoundingVolume::Box());

	return 0;
}