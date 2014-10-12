package org.omras2;
import lombok.Getter;
import lombok.Setter;

public class Status
{
	@Getter private int numFiles;
	@Getter private int dim;
	@Getter private int dudCount;
	@Getter private int nullCount;
	@Getter private int length;
	@Getter private int dataRegionSize;
	@Getter private boolean isL2Normed;
	@Getter private boolean hasPower;
	@Getter private boolean hasTimes;
	@Getter private boolean hasReferences;
}
