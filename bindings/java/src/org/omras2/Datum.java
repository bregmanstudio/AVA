package org.omras2;
import lombok.Getter;
import lombok.Setter;

public class Datum
{
	@Getter @Setter private int nvectors;
	@Getter @Setter private int dim;
	
	@Getter @Setter private double[] data = {};
	@Getter @Setter private double[] power = {};
	@Getter @Setter private double[] times = {};
}

