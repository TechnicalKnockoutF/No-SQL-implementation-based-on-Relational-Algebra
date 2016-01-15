
public class LineSegment{
	private Long id;
	private Vertices[] vert = new Vertices[2];
	private double length;
	private String segmentName;
	
	public LineSegment(Long edgeID, Vertices[] vert)
	{
		setId(edgeID);
		setVert(vert);
		setLength(getDistance());
	}

	public Long getId() {
		return id;
	}

	public void setId(Long id) {
		this.id = id;
	}

	public Vertices[] getVert() {
		return vert;
	}

	public void setVert(Vertices[] vertex) {
		this.vert = vertex;
	}

	private double getDistance()
	{
		return Math.sqrt(((vert[0].getX() - vert[1].getX())*(vert[0].getX() - vert[1].getX())) 
				+ ((vert[0].getY() - vert[1].getY())*(vert[0].getY() - vert[1].getY())));
	}

	public double getLength() {
		return length;
	}

	public void setLength(double length) {
		this.length = length;
	}

	public String getSegmentName() {
		return segmentName;
	}

	public void setSegmentName(String segmentName) {
		this.segmentName = segmentName;
	}
}
