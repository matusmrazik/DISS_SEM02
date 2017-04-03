#ifndef WEIGHTED_STATISTIC_HPP
#define WEIGHTED_STATISTIC_HPP

class weighted_statistic
{
public:
	weighted_statistic();
	~weighted_statistic();

	void add(double value, double weight);
	void clear();

	double mean() const;
	double mean(double weight) const;

private:
	double _sum;
	double _cur_value;
	double _cur_weight;
};

#endif // WEIGHTED_STATISTIC_HPP
