package org.omras2;
import lombok.Getter;
import lombok.Setter;

public class Query
{
	public enum Accumulation { DB, PER_TRACK, ONE_TO_ONE };
	public enum Distance { DOT, EUCLIDEAN_NORMED, EUCLIDEAN };

	@Getter @Setter private int seqLength;
	@Getter @Setter private int seqStart;
	@Getter @Setter private int npoints;
	@Getter @Setter private int ntracks;
	@Getter @Setter private int hopSize;
	
	@Getter @Setter private boolean exhaustive;
	@Getter @Setter private boolean hasFalsePositives;
	@Getter @Setter private Accumulation accumulation;
	@Getter @Setter private Distance distance;

	@Getter @Setter private String[] includeKeys = {};
	@Getter @Setter private String[] excludeKeys = {};

	@Getter @Setter private double radius;
	@Getter @Setter private double absThres;
	@Getter @Setter private double relThres;
	@Getter @Setter private double durRatio;

	@Getter @Setter private Datum datum;

	public Query()
	{
		accumulation = Accumulation.PER_TRACK;
		distance = Distance.EUCLIDEAN_NORMED;
		datum = new Datum();
	}

}
