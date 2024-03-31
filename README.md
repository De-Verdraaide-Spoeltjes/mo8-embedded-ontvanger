# MO8 - Embedded - Ontvanger

## Vitis configuratie
1. Download de laatste hardware file (.xsa) van de MO8 Digitaal Ontvanger repository.
2. Maak in Vitis een Platform Project gebaseerd op deze hardware file.
3. Maak een nieuw Application Project in Vitis. Gebruik hierbij het platform project wat je aangemaakt hebt.
Kies voor een Hello world application.
4. Sluit Vitis af.
5. Ga naar de map van je project, en verwijder de map 'src' en het bestand '.gitignore'.
6. Clone deze repository. Verplaats de inhoud van de repository naar de map van je project, zodat de nieuwe 'src' map op dezelfde plek terecht komt. Vergeet de '.git' map niet!
7. Open Vitis en open je project. Kijk of alles werkt.

## Nieuwe hardware file importeren
Doe dit alleen als het nodig is, want dit kan een hoop gezeik opleveren.
1. Verwijder het Platform project uit Vitis. Zorg dat "Delete on disk" aangevinkt is.
2. Maak een nieuw Platform project aan met de nieuwe hardware file (.xsa).
3. Open de project instellingen van het Application project (.prj file). Verander het platform, en selecteer het nieuwe Platform project.
4. Build het project opnieuw.
5. Als het kapot gaat, het hele project weggooien en opnieuw opbouwen. Zorg dat je geen openstaande wijzigingen hebt in de embedded repository!