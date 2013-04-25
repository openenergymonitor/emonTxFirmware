void print_to_serial()
{
  Serial.print(emontx.realPower);
  Serial.print(" ");
  Serial.print(ct1.apparentPower);
  Serial.print(" ");
  Serial.print(ct1.powerFactor);
  Serial.print(" ");
  Serial.print(ct1.Vrms);
  Serial.print(" ");
    
  Serial.print(emontx.T1);
  Serial.print(" ");
  Serial.print(emontx.T2);
  Serial.print(" ");  
  Serial.print(emontx.T3);  
  Serial.print(" ");  
  Serial.println(emontx.T4); 
  delay(100);
}
